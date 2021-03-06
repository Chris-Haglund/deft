// Deft is a density functional package developed by the research
// group of Professor David Roundy
//
// Copyright 2010 The Deft Authors
//
// Deft is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// You should have received a copy of the GNU General Public License
// along with deft; if not, write to the Free Software Foundation,
// Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
//
// Please see the file AUTHORS for a list of authors.

#include <stdio.h>
#include <time.h>
#include "Functionals.h"
#include "LineMinimizer.h"

const double my_kT = 1e-3; // room temperature in Hartree
const double ngas = 1.14e-7; // vapor density of water
const double nliquid = 4.9388942e-3; // density of liquid water
const double mu = -my_kT*log(ngas);
const double Veff_liquid = -my_kT*log(nliquid);

// Here we set up the lattice.
Lattice lat(Cartesian(5,0,0), Cartesian(0,5,0), Cartesian(0,0,5));
double resolution = 0.2;
GridDescription gd(lat, resolution);

// And the functional...
const double interaction_energy_scale = 0.01;
Functional attraction = GaussianPolynomial(-interaction_energy_scale/nliquid/nliquid/2, 0.5, 2);
Functional repulsion = GaussianPolynomial(interaction_energy_scale/nliquid/nliquid/nliquid/nliquid/4, 0.125, 4);
Functional f0 = ChemicalPotential(mu) + attraction + repulsion;
Functional n = EffectivePotentialToDensity();
Functional f = IdealGasOfVeff + f0(n);

Grid potential(gd);
Grid external_potential(gd, 1e-3/nliquid*(-0.2*r2(gd)).cwise().exp()); // repulsive bump

Functional ff = IdealGasOfVeff + (f0 + ExternalPotential(external_potential))(n);


int test_minimizer(const char *name, Minimizer min, Grid *pot, double fraccuracy=1e-3) {
  clock_t start = clock();
  printf("\n************");
  for (unsigned i=0;i<strlen(name);i++) printf("*");
  printf("\n* Testing %s *\n", name);
  for (unsigned i=0;i<strlen(name);i++) printf("*");
  printf("************\n\n");

  const double true_energy = -0.2674037773859;
  //const double gas_energy = -1.250000000000085e-11;

  *pot = +1e-4*((-10*r2(gd)).cwise().exp()) + 1.14*Veff_liquid*VectorXd::Ones(pot->description().NxNyNz);

  while (min.improve_energy(false)) fflush(stdout);

  min.print_info();
  printf("Minimization took %g seconds.\n", (clock() - double(start))/CLOCKS_PER_SEC);
  printf("fractional energy error = %g\n", (min.energy() - true_energy)/fabs(true_energy));
  if (fabs((min.energy() - true_energy)/true_energy) > fraccuracy) {
    printf("FAIL: Error in the energy is too big!\n");
    return 1;
  }
  if (min.energy() < true_energy) {
    printf("FAIL: Sign of error is wrong!!!\n");
    return 1;
  }
  return 0;
}

int main(int, char **argv) {
  int retval = 0;
  attraction.set_name("attraction");
  repulsion.set_name("repulsion");

  {
    Grid test_density(gd, EffectivePotentialToDensity()(my_kT, gd, -1e-4*(-2*r2(gd)).cwise().exp()
                                                        + mu*VectorXd::Ones(gd.NxNyNz)));
    potential = +1e-4*((-10*r2(gd)).cwise().exp()) + 1.14*Veff_liquid*VectorXd::Ones(gd.NxNyNz);
    retval += f.run_finite_difference_test("simple liquid", my_kT, potential);
    
    retval += attraction.run_finite_difference_test("quadratic", my_kT, test_density);
    retval += repulsion.run_finite_difference_test("repulsive", my_kT, test_density);
  }

  Minimizer downhill = MaxIter(300, Downhill(ff, gd, my_kT, &potential));
  potential.setZero();
  retval += test_minimizer("Downhill", downhill, &potential, 1e-9);

  Minimizer pd = MaxIter(300, PreconditionedDownhill(ff, gd, my_kT, &potential));
  potential.setZero();
  retval += test_minimizer("PreconditionedDownhill", pd, &potential, 1e-9);

  Minimizer steepest = MaxIter(150, SteepestDescent(ff, gd, my_kT, &potential, QuadraticLineMinimizer));
  potential.setZero();
  retval += test_minimizer("SteepestDescent", steepest, &potential, 1e-9);

  Minimizer psd = MaxIter(150, PreconditionedSteepestDescent(ff, gd, my_kT, &potential, QuadraticLineMinimizer));
  potential.setZero();
  retval += test_minimizer("PreconditionedSteepestDescent", psd, &potential, 1e-9);

  Minimizer cg = MaxIter(150, ConjugateGradient(ff, gd, my_kT, &potential, QuadraticLineMinimizer));
  potential.setZero();
  retval += test_minimizer("ConjugateGradient", cg, &potential, 1e-11);

  Minimizer pcg = MaxIter(150, PreconditionedConjugateGradient(ff, gd, my_kT, &potential, QuadraticLineMinimizer));
  potential.setZero();
  retval += test_minimizer("PreconditionedConjugateGradient", pcg, &potential, 1e-11);

  
  potential = +1e-4*((-10*r2(gd)).cwise().exp()) + 1.14*mu*VectorXd::Ones(gd.NxNyNz);

  if (retval == 0) {
    printf("\n%s passes!\n", argv[0]);
  } else {
    printf("\n%s fails %d tests!\n", argv[0], retval);
    return retval;
  }
}
