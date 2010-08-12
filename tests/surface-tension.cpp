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

const double kT = 1e-3; // room temperature in Hartree
const double ngas = 1.14e-7; // vapor density of water
const double nliquid = 4.9388942e-3; // density of liquid water
const double mu = -kT*log(ngas);
const double Veff_liquid = -kT*log(nliquid);

// Here we set up the lattice.
Lattice lat(Cartesian(0.2,0,0), Cartesian(0,0.2,0), Cartesian(0,0,20));
double resolution = 0.2/2;
GridDescription gd(lat, resolution);

// And the functional...
const double interaction_energy_scale = 2e-4;
Functional attraction = GaussianPolynomial(-interaction_energy_scale/nliquid/nliquid/2, 0.5, 2);
Functional repulsion = GaussianPolynomial(interaction_energy_scale/nliquid/nliquid/nliquid/nliquid/4, 0.125, 4);
Functional f0 = IdealGas(kT) + ChemicalPotential(mu) + attraction + repulsion;
Functional f = compose(f0, EffectivePotentialToDensity(kT));

Grid external_potential(gd);
Grid potential(gd);

Functional ff;

int test_minimizer(const char *name, Minimizer *min, int numiters, double fraccuracy=1e-3) {
  clock_t start = clock();
  printf("\n************");
  for (unsigned i=0;i<strlen(name);i++) printf("*");
  printf("\n* Testing %s *\n", name);
  for (unsigned i=0;i<strlen(name);i++) printf("*");
  printf("************\n\n");

  potential = Veff_liquid*VectorXd::Ones(gd.NxNyNz);

  for (int i=0;i<numiters && min->improve_energy(false);i++) {
    fflush(stdout);
  }
  min->print_info();
  min->minimize(f, gd);
  for (int i=0;i<numiters && min->improve_energy(false);i++) {
    //printf("Without external potential...\n");
    fflush(stdout);
  }
  min->print_info();  

  const double Einterface = f(potential);
  double Ninterface = 0;
  {
    Grid density(gd, EffectivePotentialToDensity(kT)(gd, potential));
    for (int i=0;i<gd.NxNyNz;i++) Ninterface += density[i]*gd.dvolume;
  }
  printf("Minimization took %g seconds.\n", (clock() - double(start))/CLOCKS_PER_SEC);

  Grid gas(gd, mu*VectorXd::Ones(gd.NxNyNz));
  min->minimize(f, gd, &gas);
  // The following sometimes fails with NaNs if it's run too long, for
  // reasons I don't understand.
  for (int i=0;i<numiters && min->improve_energy(false);i++) {
    //printf("GAS\n");
    fflush(stdout);
  }
  const double Egas = f(gas);
  double Ngas = 0;
  {
    Grid density(gd, EffectivePotentialToDensity(kT)(gd, gas));
    for (int i=0;i<gd.NxNyNz;i++) Ngas += density[i]*gd.dvolume;
  }
  min->print_info();
  printf("gas energy is %g\n", f(gas));
  printf("Minimization took %g seconds.\n", (clock() - double(start))/CLOCKS_PER_SEC);

  double minpot = 1e100;
  for (int i=0;i<gd.NxNyNz;i++)
    if (potential[i] < minpot) minpot = potential[i];
  Grid liquid(gd, minpot*VectorXd::Ones(gd.NxNyNz));
  min->minimize(f, gd, &liquid);
  for (int i=0;i<numiters && min->improve_energy(false);i++) {
    //printf("LIQUID\n");
    fflush(stdout);
  }
  min->print_info();
  printf("Minimization took %g seconds.\n", (clock() - double(start))/CLOCKS_PER_SEC);
  const double Eliquid = f(liquid);
  double Nliquid = 0;
  {
    Grid density(gd, EffectivePotentialToDensity(kT)(gd, liquid));
    for (int i=0;i<gd.NxNyNz;i++) Nliquid += density[i]*gd.dvolume;
  }

  printf("\n\n");
  printf("interface energy is %.15g\n", Einterface);
  printf("gas energy is %.15g\n", Egas);
  printf("liquid energy is %.15g\n", Eliquid);
  printf("Ninterface/liquid/gas = %g/%g/%g\n", Ninterface, Nliquid, Ngas);
  const double X = Ninterface/Nliquid; // Fraction of volume effectively filled with liquid.
  printf("X is %g\n", X);
  const double true_surface_tension = 1.93512e-06;
  const double surface_tension = (Einterface - Eliquid*X - Egas*(1-X))/2/0.2/0.2;
  printf("surface tension is %g\n", surface_tension);


  printf("fractional surface tension error = %g\n", (surface_tension - true_surface_tension)/fabs(true_surface_tension));
  if (fabs((surface_tension - true_surface_tension)/true_surface_tension) > fraccuracy) {
    printf("Error in the surface tension is too big!\n");
    return 1;
  }
  return 0;
}

double forcer(Cartesian r) {
  const double z = r.dot(Cartesian(0,0,1));
  return 0.2/nliquid*(exp(-0.5*(z-5)*(z-5)) - exp(-(z-15)*(z-15)));
}

int main(int, char **argv) {
  external_potential.Set(forcer);
  Functional f1 = f0 + ExternalPotential(external_potential);
  ff = compose(f1, EffectivePotentialToDensity(kT));

  Grid test_density(gd, EffectivePotentialToDensity(kT)(gd, -1e-4*(-2*external_potential.r2()).cwise().exp()
                                                        + mu*VectorXd::Ones(gd.NxNyNz)));

  // The following is for debugging our simple liquid...
  //printf("mu is %g\n", mu);
  //printf("Veff_liquid is %g\n", Veff_liquid);
  //printf("%12s\t%12s\t%12s\n", "Veff", "n", "energy");
  //for (double v=0.5*Veff_liquid; v<mu+5*kT; v*=1.05) {
  //  printf("%12g\t%12g\t%12g\n", v, EffectivePotentialToDensity(kT)(v), f(v));
  //}

  int retval = 0;

  potential = +1e-4*((-10*potential.r2()).cwise().exp()) + 1.14*mu*VectorXd::Ones(gd.NxNyNz);
  retval += ff.run_finite_difference_test("simple liquid", potential);
  fflush(stdout);

  retval += attraction.run_finite_difference_test("quadratic", test_density);
  fflush(stdout);
  retval += repulsion.run_finite_difference_test("repulsive", test_density);
  fflush(stdout);

  {
    Minimizer pd = PreconditionedDownhill(ff, gd, &potential, 1e-7);
    potential.setZero();
    retval += test_minimizer("PreconditionedDownhill", &pd, 200, 1e-4);

    Grid density(gd, EffectivePotentialToDensity(kT)(gd, potential));
    density.epsNativeSlice("PreconditionedDownhill.eps", Cartesian(0,0,20), Cartesian(0.1,0,0),
                           Cartesian(0,0,0));

    retval += attraction.run_finite_difference_test("quadratic", density);
    retval += repulsion.run_finite_difference_test("repulsive", density);
  }

  Minimizer psd = PreconditionedSteepestDescent(ff, gd, &potential, QuadraticLineMinimizer, 1e-7);
  potential.setZero();
  retval += test_minimizer("PreconditionedSteepestDescent", &psd, 400, 1e-4);

  if (retval == 0) {
    printf("\n%s passes!\n", argv[0]);
  } else {
    printf("\n%s fails %d tests!\n", argv[0], retval);
    return retval;
  }
}