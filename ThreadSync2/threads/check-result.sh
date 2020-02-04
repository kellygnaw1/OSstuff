#!/bin/bash
#verify program output against known good results
prog=$1
test=$2
resultdir=$3
rundir=$(dirname "$0")
[ -z "$resultdir" ] && resultdir="$rundir/results"
[ -x "$prog" ] || { echo -e "unknown progam $prog\nUsage: $0 <progname> <test> [<resultdir>]"; exit 1; }
[ -d "$resultdir" ] || { echo -e "Can't find result directory '$resultdir'\nUsage: $0 <progname> <test> [<resultdir>]"; exit 1; }

"$PWD/$prog" -q $test | diff "$resultdir/$prog-$test.txt" -
