#!/bin/bash
#check whether test is successful by grepping output
prog=$1
test=$2
rundir=$(dirname "$0")
. "$rundir/test-vars.sh"
[ -x "$prog" ] || { echo -e "unknown progam $prog\nUsage: $0 <progname> <test>"; exit 1; }

"$PWD/$prog" $numthreads $numkeys "$test" $numiter | grep -q "All tests passed"
