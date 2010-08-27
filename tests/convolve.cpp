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
#include "Grid.h"
#include "Functionals.h"

double gaussian(Cartesian r) {
  const Cartesian center(1, 0, 0);
  const Cartesian dr(r - center);
  // This is smooth so that we won't suffer from oscillations due to
  // aliasing effects.  We might also evaluate the convolution
  // function in real space and do a DFT on it to get the convolution
  // in fourier space.  This would avoid the oscillations, but would
  // require that we store the convolution function on a grid.
  return 4000*exp(-50*(dr*dr));
}

int main(int, char **argv) {
  Lattice lat(Cartesian(0,5,5), Cartesian(5,0,5), Cartesian(5,5,0));
  Cartesian plotcorner(-5, -5, 0), plotx(10,0,0), ploty(0,10,0);
  int resolution = 100;
  GridDescription gd(lat, resolution, resolution, resolution);
  Grid foo(gd), bar(gd);
  printf("Running Set(gaussian)...\n");
  foo.Set(gaussian);
  const double integrate_foo = integrate(foo);
  printf("Original integrates to %.15g\n", integrate_foo);
  printf("Original Maximum is %g\n", foo.maxCoeff());
  int retval = 0;
  if (integrate_foo < 0) {
    printf("Integral of original is negative (which may throw off tests):  %g\n", integrate_foo);
    retval++;
  }
  foo.epsNativeSlice("unblurred.eps", plotx, ploty, plotcorner);

  printf("Running Gaussian(10)...\n");
  bar = Gaussian(10)(foo);
  printf("Gaussian(10) integrates to %.15g\n", integrate(bar));
  if (fabs(integrate(bar)/integrate_foo-1) > 1e-6) {
    printf("Error in Gaussian(10) is too large:  %g\n", integrate(bar)/integrate_foo-1);
    retval++;
  }
  bar.epsNativeSlice("gaussian-width-10.eps", plotx, ploty, plotcorner);

  printf("Gaussian(1) integrates to %g\n", integrate(bar));
  printf("Running Gaussian(1)...\n");
  bar = Gaussian(1)(foo);
  if (fabs(integrate(bar)/integrate_foo-1) > 1e-6) {
    printf("Error in Gaussian(1) is too large:  %g\n", integrate(bar)/integrate_foo-1);
    retval++;
  }
  bar.epsNativeSlice("gaussian-width-1.eps", plotx, ploty, plotcorner);

  printf("Running StepConvolve(1)...\n");
  bar = StepConvolve(1)(foo);
  printf("StepConvolve(1) integrates to %.15g\n", integrate(bar));
  printf("StepConvolve(1) Maximum is %g\n", bar.maxCoeff());
  if (fabs(bar.maxCoeff()/integrate_foo-1) > 1e-6) {
    printf("Max of StepConvolve(1) is wrong:  %g\n", bar.maxCoeff()/integrate_foo - 1);
    retval++;
  }
  const double fourpiover3 = 4*M_PI/3;
  if (fabs((integrate(bar)/integrate_foo-fourpiover3)/fourpiover3) > 1e-6) {
    printf("Integral of StepConvolve(1) is wrong:  %g\n",
           (integrate(bar)/integrate_foo-fourpiover3)/fourpiover3);
    retval++;
  }
  bar.epsNativeSlice("step-1.eps", plotx, ploty, plotcorner);
  retval += integrate(StepConvolve(1)).run_finite_difference_test("integrate StepConvolve(1)", foo);

  printf("Running StepConvolve(2)...\n");
  bar = StepConvolve(2)(foo);
  printf("StepConvolve(2) Maximum is %g\n", bar.maxCoeff());
  printf("StepConvolve(2) integrates to %.15g\n", integrate(bar));
  if (fabs((integrate(bar)/integrate_foo-fourpiover3*8)/fourpiover3/8) > 1e-6) {
    printf("Integral of StepConvolve(2) is wrong:  %g\n", (integrate(bar)/integrate_foo-fourpiover3*8)/fourpiover3/8);
    retval++;
  }
  if (fabs((bar.maxCoeff()-integrate_foo)/integrate_foo) > 1e-6) {
    printf("Max of StepConvolve(2) is wrong:  %g\n", (bar.maxCoeff()-integrate_foo)/integrate_foo);
    retval++;
  }
  bar.epsNativeSlice("step-2.eps", plotx, ploty, plotcorner);
  retval += integrate(StepConvolve(2)).run_finite_difference_test("integrate StepConvolve(2)", foo);

  printf("Running StepConvolve(3)...\n");
  bar = StepConvolve(3)(foo);
  printf("StepConvolve(3) Maximum is %g\n", bar.maxCoeff());
  printf("StepConvolve(3) integrates to %.15g\n", integrate(bar));
  if (fabs((integrate(bar)/integrate_foo-fourpiover3*27)/fourpiover3/27) > 1e-6) {
    printf("Integral of StepConvolve(3) is wrong:  %g\n",
           (integrate(bar)/integrate_foo-fourpiover3*27)/fourpiover3/27);
    retval++;
  }
  if (fabs((bar.maxCoeff()-integrate_foo)/integrate_foo) > 1e-6) {
    printf("Max of StepConvolve(3) is wrong:  %g\n", (bar.maxCoeff()-integrate_foo)/integrate_foo);
    retval++;
  }
  bar.epsNativeSlice("step-3.eps", plotx, ploty, plotcorner);
  retval += integrate(StepConvolve(3)).run_finite_difference_test("integrate StepConvolve(3)", foo);


  const double fourpi = 4*M_PI;
  printf("Running ShellConvolve(1)...\n");
  bar = ShellConvolve(1)(foo);
  printf("ShellConvolve(1) integrates to %.15g\n", integrate(bar));
  printf("ShellConvolve(1) Maximum is %g\n", bar.maxCoeff());
  if (fabs((integrate(bar)/integrate_foo-fourpi)/fourpi) > 1e-6) {
    printf("Integral of ShellConvolve(1) is wrong:  %g\n",
           (integrate(bar)/integrate_foo-fourpi)/fourpi);
    retval++;
  }
  bar.epsNativeSlice("shell-1.eps", plotx, ploty, plotcorner);
  retval += integrate(ShellConvolve(1)).run_finite_difference_test("integrate ShellConvolve(1)", foo);

  printf("Running ShellConvolve(3)...\n");
  bar = ShellConvolve(3)(foo);
  printf("ShellConvolve(3) Maximum is %g\n", bar.maxCoeff());
  printf("ShellConvolve(3) integrates to %.15g\n", integrate(bar));
  if (fabs((integrate(bar)/integrate_foo-fourpi*9)/fourpi/9) > 1e-6) {
    printf("Integral of ShellConvolve(3) is wrong:  %g\n",
           (integrate(bar)/integrate_foo-fourpi*9)/fourpi/9);
    retval++;
  }
  bar.epsNativeSlice("shell-3.eps", plotx, ploty, plotcorner);
  retval += integrate(ShellConvolve(3)).run_finite_difference_test("integrate ShellConvolve(3)", foo);

  printf("Running yShellConvolve(1)...\n");
  bar = yShellConvolve(1)(foo);
  printf("yShellConvolve(1) integrates to %.15g\n", integrate(bar));
  printf("yShellConvolve(1) Maximum is %g\n", bar.maxCoeff());
  if (fabs(integrate(bar)/integrate_foo) > 1e-6) {
    printf("Integral of yShellConvolve(1) is wrong:  %g\n",
           integrate(bar)/integrate_foo-fourpi);
    retval++;
  }
  bar.epsNativeSlice("y-shell-1.eps", plotx, ploty, plotcorner);
  FieldFunctional ysh = yShellConvolve(1), sh = ShellConvolve(1);
  retval += integrate(sqr(ysh)).run_finite_difference_test("integrate ysh^2", foo);
  retval += integrate(ysh*ysh).run_finite_difference_test("integrate ysh*ysh", foo);
  retval += integrate(ysh*ysh + sh).run_finite_difference_test("integrate ysh*ysh + sh", foo);
  retval += integrate(-1*ysh*ysh).run_finite_difference_test("integrate -1*ysh*ysh", foo);
  retval += (-1*integrate(ysh*ysh)).run_finite_difference_test("-1* integrate ysh*ysh", foo);

  printf("Running xShellConvolve(3)...\n");
  bar = xShellConvolve(3)(foo);
  printf("xShellConvolve(3) Maximum is %g\n", bar.maxCoeff());
  printf("xShellConvolve(3) integrates to %.15g\n", integrate(bar));
  if (fabs(integrate(bar)/integrate_foo) > 1e-6) {
    printf("Integral of xShellConvolve(3) is wrong:  %g\n",
           integrate(bar)/integrate_foo);
    retval++;
  }
  bar.epsNativeSlice("x-shell-3.eps", plotx, ploty, plotcorner);

  printf("Running xyShellConvolve(1)...\n");
  bar = xyShellConvolve(1)(foo);
  printf("xyShellConvolve(1) integrates to %.15g\n", integrate(bar));
  printf("xyShellConvolve(1) Maximum is %g\n", bar.maxCoeff());
  if (fabs(integrate(bar)/integrate_foo) > 1e-6) {
    printf("Integral of xyShellConvolve(1) is wrong:  %g\n",
           integrate(bar)/integrate_foo-fourpi);
    retval++;
  }
  bar.epsNativeSlice("xy-shell-1.eps", plotx, ploty, plotcorner);

  printf("Running xxShellConvolve(2)...\n");
  bar = xxShellConvolve(2)(foo);
  printf("xxShellConvolve(2) integrates to %.15g\n", integrate(bar));
  printf("xxShellConvolve(2) Maximum is %g\n", bar.maxCoeff());
  if (fabs(integrate(bar)/integrate_foo + fourpi*4/3) > 1e-6) {
    printf("Integral of xxShellConvolve(2) is wrong:  %g\n",
           integrate(bar)/integrate_foo+fourpi*4/3);
    retval++;
  }
  bar.epsNativeSlice("xx-shell-2.eps", plotx, ploty, plotcorner);

  printf("Running zxShellConvolve(3)...\n");
  FieldFunctional zxsh = zxShellConvolve(3);
  bar = zxsh(foo);
  printf("zxShellConvolve(3) Maximum is %g\n", bar.maxCoeff());
  printf("zxShellConvolve(3) integrates to %.15g\n", integrate(bar));
  if (fabs(integrate(bar)/integrate_foo) > 1e-6) {
    printf("Integral of zxShellConvolve(3) is wrong:  %g\n",
           integrate(bar)/integrate_foo);
    retval++;
  }
  bar.epsNativeSlice("zx-shell-3.eps", plotx, ploty, Cartesian(-5,-5,0.5));
  retval += integrate(sqr(sqr(zxsh))).run_finite_difference_test("integrate zxsh^2", foo);

  if (retval == 0) printf("\n%s passes!\n", argv[0]);
  else printf("\n%s fails %d tests!\n", argv[0], retval);
  return retval;
}