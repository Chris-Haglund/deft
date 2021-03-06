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
set output 'figs/sphere-radial-06-04.eps'

set key noauto

set multiplot

set size 1,1          # The first plot (host plot)
set origin 0,0
#set title 'Density and contact densities'
set xlabel 'radial position / hard sphere radius'
set ylabel 'filling fraction'

set style line 1 lt 1 lw 3
set style line 2 lt 2 lw 3
set style line 3 lt 3 lw 3
set style line 4 lt 4 lw 3

plot [:] [:] \
'figs/sphere-radial-06.0-02.dat' u 1:($2*4*pi/3) title 'density' with lines ls 1, \
'figs/sphere-radial-06.0-02.dat' u 1:($4*4*pi/3) title 'contact density' with lines ls 2, \
'figs/sphere-radial-06.0-02.dat' u 1:($5*4*pi/3) title 'Fu and Wu contact density' with l ls 3, \
'figs/sphere-radial-06.0-02.dat' u 1:($5*6*pi/3) title 'contact density at sphere' with l ls 4, \
'figs/sphere-radial-06.0-06.dat' u 1:($2*4*pi/3) notitle with lines ls 1, \
'figs/sphere-radial-06.0-06.dat' u 1:($4*4*pi/3) notitle with lines ls 2, \
'figs/sphere-radial-06.0-06.dat' u 1:($5*4*pi/3) notitle with l ls 3, \
'figs/sphere-radial-06.0-06.dat' u 1:($6*4*pi/3) notitle with l ls 4, \
'figs/sphere-radial-08.0-02.dat' u 1:($2*4*pi/3) notitle with lines ls 1, \
'figs/sphere-radial-08.0-02.dat' u 1:($4*4*pi/3) notitle with lines ls 2, \
'figs/sphere-radial-08.0-02.dat' u 1:($5*4*pi/3) notitle with l ls 3, \
'figs/sphere-radial-08.0-02.dat' u 1:($6*4*pi/3) notitle with l ls 4, \
'figs/sphere-radial-08.0-08.dat' u 1:($2*4*pi/3) notitle with lines ls 1, \
'figs/sphere-radial-08.0-08.dat' u 1:($4*4*pi/3) notitle with lines ls 2, \
'figs/sphere-radial-08.0-08.dat' u 1:($5*4*pi/3) notitle with l ls 3, \
'figs/sphere-radial-08.0-08.dat' u 1:($6*4*pi/3) notitle with l ls 4
