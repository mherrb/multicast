#! /usr/bin/env -S gnuplot --persist -c

set key outside
if (ARGC == 0) {
   data='data.txt'
} else {
   data  = ARG1
}
plot for [col=1:2] data using 0:col with lines title columnheader
