#!/bin/bash
#check whether test is successful by grepping output
prog=$1
test=$2
rundir=$(dirname "$0")
[ -x "$prog" ] || { echo -e "unknown progam $prog\nUsage: $0 <progname> <test>"; exit 1; }

"$PWD/$prog" -q $test | grep -q "All tests passed"
