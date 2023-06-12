#! /usr/bin/env -S gnuplot --persist -c

set key outside
data = ARG1
print "arg : ", ARGC
print "data : ", data

if (!exists("data")) data='data.txt'
plot for [col=1:2] data using 0:col with lines title columnheader
