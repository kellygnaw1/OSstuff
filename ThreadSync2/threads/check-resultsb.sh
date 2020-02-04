#!/bin/bash
prog=$1
resultdir=$2
rundir=$(dirname "$0")
[ -z "$resultdir" ] && resultdir="$rundir/results"
[ -x "$prog" ] || { echo -e "Unknown program $prog\nUsage: $0 <progname> [<resultsdir>]"; exit 1; }
[ -d "$resultdir" ] || { echo -e "Can't find result directory '$resultdir'\nUsage: $0 <progname> [<resultdir>]"; exit 1; }

#Run each test for $prog
#For each test:
#- run the test with all output redirected to /dev/null (we just care whether the diff returned 0 or not)
#  - when test programs crash causing a backtrace, that normally bypasses redirections and still goes to tty
#  - The other version of the script gets around that with `script`, but this version has extra output sometimes when the program crashes
#- print PASS/FAIL message based on exit code of check-result script
< "$rundir/tests-$prog" xargs -n1 -I'{}' bash -c "'$rundir/check-result.sh' '$prog' {} '$resultdir' >/dev/null 2>&1 && echo '$prog -q {}: PASS' || echo '$prog -q {}: FAIL'"
