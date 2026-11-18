#!/usr/bin/env bash

# usage: ./plot.sh <filename>.csv

INPUT="$1"
TMPFILE="$(mktemp ./$1.dat)"

awk '
{
    gsub(/[()]/,"")    
    n = split($0, a, ",")
    for (i = 1; i < n; i += 2) {
        print a[i], a[i+1]
    }
    print ""
    print ""
}
' "$INPUT" > "$TMPFILE"

gnuplot << EOF
set title "Pareto Fronts"
set xlabel "X"
set ylabel "Y"
set grid

set key off
set term qt size 900,600 font "Helvetica,12" persist

plot for [i=0:*] "$TMPFILE" index i \
     using 1:2 with linespoints lw 2 pt 7 title sprintf("Front %d", i+1)
EOF