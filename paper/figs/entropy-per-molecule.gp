#!/usr/bin/gnuplot
#
# guide for line and point styles:
#
#  0  ..............  .                    broken line
#  1  --------------  +                    red
#  2  -- -- -- -- --  x                    green
#  3  -  -  -  -  -   *                    blue
#  4  ..............  empty square         magenta
#  5  __.__.__.__.__  full  square         cyan
#  6  _ . _ . _ . _   empty circle         yellow
#  7  - -  - -  - -   full  circle         black
#  8  - - -  - - -    empty up triangle    brown
#  9  - - - -  - - -  full  up triangle    grey
# 10 (1)              empty down triangle
# 11 (2)              full  down triangle
# 12 (3)              empty diamond
# 13 (4)              full  diamond
# 14 (5)              empty pentagon
# 15 (6)              full  pentagon
# 16-31               watches

set terminal postscript eps enhanced color "Helvetica" 20
set output 'figs/entropy-per-molecule.eps'

set key noauto

set multiplot

set size 1,1          # The first plot (host plot)
set origin 0,0
set title 'Entropy per molecule?'
set xlabel 'density (bohr^{-3})'
set ylabel 'Entropy (J/K)?'
set style line 1 lt 1 lw 3
set style line 2 lt 2 lw 3

JpermolK = 1.3806503e-23*6.0221e23

HperKtoJperK = 27.2117*1.6e-19 # Converts Hartree/K to J/K

molperltobohr = 6.02214179e23*1e-24/(18.8972613*18.8972613*18.8972613)  # Converts mole/liter to bohr^-3

kB = 3.16681539628059e-6 # This is Boltzmann's constant in Hartree/Kelvin

plot [:] [:] \
'figs/entropy.dat' u 1:($3/$1*HperKtoJperK) title 'T=298K' with lines ls 1, \
'figs/entropy-at-690K.dat' u 1:($3/$1*HperKtoJperK) title 'T=690K' with lines ls 2, \
'figs/pressure-298K.dat' u ($3*molperltobohr):($7/($3*molperltobohr)/ 6.02214179e23) title 'expt T=298K' with points