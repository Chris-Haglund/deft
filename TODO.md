- Implement a minimizer of some sort (e.g. EOM).

- Test minimizer on ideal gas with a given chemical potential.

- Implement an external potential.

- Test that the ideal gas has the appropriate (barometer formula)
  density in that external potential.

- Implement huge repulsion, to make sure we can look at a hard-surface
  cavity.

- Implement simple quadratic attraction functional.

- Write a program to do surface-tension calculations by putting an
  infinite barrier at one spot, and adjusting the chemical potential
  until we have a liquid-vapor interface and then setting the chemical
  potential back to its desired value and re-relaxing.  Or perhaps
  fixing the density at vapor and liquid densities at two planes in
  space?

- Implement line minimization.

- Implement steepest-descent with line minimization.

- Implement conjugate gradients minimization.

- Implement smart stop condition for minimization as I did in DFT++.

- Implement White Bear hard-sphere functional.

- Look up Fu in paper to find what 