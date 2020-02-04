#!/bin/bash
mkdir -p results
touch results
for prog; do
  [ -x "$PWD/$prog" ] || { echo -e "unknown progam $PWD/$prog\nUsage: $0 <progname1> ..."; exit 1; }
  [ -f "tests-$prog" ] || { echo -e "missing test list tests-$prog for progam $prog\nCreate a file named tests-$prog with the list of tests to run against $prog"; exit 1; }
  < "tests-$prog" xargs -n1 -I'{}' sh -c "'$PWD/$prog' -q {} > 'results/${prog}-{}.txt'"
done
exit 0
