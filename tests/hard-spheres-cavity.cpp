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
#include "utilities.h"

const double kT = water_prop.kT; // room temperature in Hartree
const double R = 2.7;
const double eta_one = 3.0/(4*M_PI*R*R*R);
const double diameter_cubed = 1/(8*R*R*R);
const double nliquid = 0.324*eta_one;
const double mu = -integrate(HardSpheres(R, kT) + IdealGas(kT)).grad(nliquid);

// Here we set up the lattice.
const double rcav = R+R; // 11.8*R+R;
const double rmax = rcav*2;
Lattice lat(Cartesian(0,rmax,rmax), Cartesian(rmax,0,rmax), Cartesian(rmax,rmax,0));
//Lattice lat(Cartesian(1.4*rmax,0,0), Cartesian(0,1.4*rmax,0), Cartesian(0,0,1.4*rmax));
GridDescription gd(lat, 0.2);

// And the functional...
FieldFunctional f0 = HardSpheres(R, kT) + IdealGas(kT) + ChemicalPotential(mu);
FieldFunctional f0wb = HardSpheresWB(R, kT);
FieldFunctional f0rf = HardSpheresRF(R, kT);
FieldFunctional n = EffectivePotentialToDensity(kT);
FieldFunctional f = f0(n);

Grid external_potential(gd);
Grid potential(gd);
Functional ff;

int test_minimizer(const char *name, Minimizer min, int numiters, double fraccuracy=1e-3) {
  clock_t start = clock();
  printf("\n************");
  for (unsigned i=0;i<strlen(name);i++) printf("*");
  printf("\n* Testing %s *\n", name);
  for (unsigned i=0;i<strlen(name);i++) printf("*");
  printf("************\n\n");

  potential = external_potential + 0.005*VectorXd::Ones(gd.NxNyNz);

  for (int i=0;i<numiters && min.improve_energy(true);i++) {
    fflush(stdout);
  }
  min.print_info();
  printf("Minimization took %g seconds.\n", (clock() - double(start))/CLOCKS_PER_SEC);

  const double true_energy = -0.0389337921975581;
  //const double true_N = 0.376241423570245;

  int retval = 0;
  double energy = min.energy();
  printf("Energy is %.15g\n", energy);
  if (energy < true_energy) {
    printf("FAIL: Energy is less than the true energy by %g!\n", true_energy - energy);
    retval++;
  }
  if (fabs(energy/true_energy - 1) > fraccuracy) {
    printf("FAIL: Error in the energy is too big: %g\n", (energy - true_energy)/fabs(true_energy));
    retval++;
  }

  double N = 0;
  {
    Grid density(gd, EffectivePotentialToDensity(kT)(gd, potential));
    for (int i=0;i<gd.NxNyNz;i++) N += density[i]*gd.dvolume;
  }
  N = N;
  printf("N is %.15g\n", N);
  //if (fabs(N/true_N - 1) > 10*fraccuracy) {
  //  printf("FAIL: Error in N is too big: %g\n", N/true_N - 1);
  //  retval++;
  //}
  return retval;
}

double notincavity(Cartesian r) {
  const double rad2 = r.dot(r);
  if (rad2 < rcav*rcav) {
    return 0;
  } else {
    return 1;
  }
}

double incavity(Cartesian r) {
  return 1 - notincavity(r);
}

int main(int, char **argv) {
  external_potential.Set(incavity);
  external_potential *= 1e9;
  external_potential.epsNativeSlice("external.eps", Cartesian(2*rmax,0,0), Cartesian(0,2*rmax,0), Cartesian(-rmax,-rmax,0));
  external_potential.epsRadial1d("external-radial.eps", 0, rmax, 1, R, "Good fun!");
  Grid constraint(gd);
  constraint.Set(notincavity);
  ff = constrain(constraint, integrate(f0(n)));

  int retval = 0;

  {
    potential = external_potential + 0.005*VectorXd::Ones(gd.NxNyNz);
    retval += integrate(f0wb(n)).run_finite_difference_test("white bear functional", potential);
    retval += integrate(f0rf(n)).run_finite_difference_test("rosenfeld functional", potential);
  }

  {
    Minimizer pd = Precision(0, PreconditionedConjugateGradient(ff, gd, &potential, QuadraticLineMinimizer));
    //Minimizer pd = Precision(0, ConjugateGradient(ff, gd, &potential, QuadraticLineMinimizer));
    retval += test_minimizer("PreconditionedConjugateGradient", pd, 10, 1e-5);

    //potential = external_potential + mu*VectorXd::Ones(gd.NxNyNz);
    Grid density(gd, EffectivePotentialToDensity(kT)(gd, potential));
    density.epsNativeSlice("cavity-density.eps", Cartesian(2*rmax,0,0), Cartesian(0,2*rmax,0), Cartesian(-rmax,-rmax,0));
    density.epsRadial1d("cavity-radial-density.eps", 0, rmax, nliquid, R, "Density scaled by nliquid");
    //density.epsNativeSlice("PreconditionedConjugateGradient.eps", Cartesian(0,0,zmax), Cartesian(1,0,0),
    //                       Cartesian(0,0,0));
    //density.epsNative1d("hard-wall-plot.eps", Cartesian(0,0,0), Cartesian(0,0,zmax), diameter_cubed, R, "Y axis: n*8*R^3, x axis: R");

    Grid grad(gd);
    grad.setZero();
    ff.grad(potential, &grad);
    //grad.epsNative1d("hard-wall-grad.eps", Cartesian(0,0,0), Cartesian(0,0,zmax), 1, R);
 
    retval += constrain(constraint, integrate(f0wb)).run_finite_difference_test("white bear functional", density, &grad);
    retval += constrain(constraint, integrate(f0rf)).run_finite_difference_test("rosenfeld functional", density, &grad);
  }

  //Minimizer psd = PreconditionedSteepestDescent(ff, gd, &potential, QuadraticLineMinimizer, 1e-4);
  //retval += test_minimizer("PreconditionedSteepestDescent", psd, 20, 1e-4);

  //Minimizer cg = ConjugateGradient(ff, gd, &potential, QuadraticLineMinimizer);
  //retval += test_minimizer("ConjugateGradient", cg, 3000, 3e-5);

  if (retval == 0) {
    printf("\n%s passes!\n", argv[0]);
  } else {
    printf("\n%s fails %d tests!\n", argv[0], retval);
    return retval;
  }
}