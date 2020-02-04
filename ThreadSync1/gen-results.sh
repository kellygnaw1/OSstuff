#!/bin/bash
#generate results for a set of programs for all test cases
#- NOTE: unlike check-result*, gen-result by default places results in ./results instead of $rundir/results
rundir=$(dirname "$0")
. "$rundir/test-vars.sh"
resultdir="results"
mkdir -p "$resultdir"
touch "$resultdir" #update directory timestamp if it already exists
for prog; do
  [ -x "$PWD/$prog" ] || { echo -e "unknown progam $PWD/$prog\nUsage: $0 <progname1> ..."; exit 1; }
  [ -f "tests-$prog" ] || { echo -e "missing test list tests-$prog for progam $prog\nCreate a file named tests-$prog with the list of tests to run against $prog"; exit 1; }
  < "tests-$prog" xargs -n1 -I'{}' sh -c "'$PWD/$prog' $numthreads $numkeys {} $numiter > '$resultdir/${prog}-{}.txt'"
done
exit 0
