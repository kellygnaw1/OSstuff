#!/bin/bash
prog=$1
rundir=$(dirname "$0")
[ -x "$prog" ] || { echo -e "Unknown program $prog\nUsage: $0 <progname>"; exit 1; }

script -q -e -c 'true' /dev/null >/dev/null 2>&1 || { echo "It looks like your version of script is older and missing some options. Try make testb or ./test-resultsb.sh instead"; exit 1; }

#Run each test for $prog
#For each test:
#- run the test with all output redirected to /dev/null (we just care whether the grep returned 0 or not)
#  - when test programs crash causing a backtrace, that normally bypasses redirections and still goes to tty, I use the script command to get around that (and capture all output including backtraces)
#- print PASS/FAIL message based on exit code of test-result script
< "$rundir/tests-$prog" xargs -n1 -I'{}' bash -c "script -q -e -c \"'$rundir/test-result.sh' '$prog' {}\" /dev/null >/dev/null && echo '$prog -q {}: PASS' || echo '$prog -q {}: FAIL'"
