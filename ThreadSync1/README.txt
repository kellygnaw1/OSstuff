The list of files in the tar ball.

=== FILES TO SUBMIT ===

1. rwlock.h
This file contains declaration for RW lock.
Need to add members for synchronization.

2. rwlock.cc
This file contains definitions for RW lock
Need to add code for synchronization.

=== FILES TO MODIFY FOR LOCAL TESTING ===

3. hashchain.h
This file contains class specifications for the hashmap implementations.

4. hashchain.cc
This file contains HashMap implementations.

5a. ptest-simple.cc
This is a simple test program which tests synchronized access to HashMap.
You can add additional tests here to aid in debugging

5b. ptest.cc
This is the comprehensive test program which tests synchronized access to HashMap, used by our
scipe on GradeScope.

6. Makefile
This is used to produce the binaries and run tests.
Comprehensive tests:
- phashchain    - no locking code (just the thread-unsafe original version)
- phashfine     - fine-grained mutex synchronization 
- phashfinerw   - fine-grained rwlock synchronization 

Simple tests:
- phashchain-simple    - no locking code (just the thread-unsafe original version)
- phashfine-simple     - fine-grained mutex synchronization 
- phashfinerw-simple   - fine-grained rwlock synchronization 

Testing targets:
- test-<prog>   - run set of tests against <prog> from file tests-<prog>
- test          - run standard tests against each program
- testb-<prog>  - run set of tests against <prog> from file tests-<prog> (see test-resultsb.sh below)
- testb         - run standard tests against each program (see test-resultsb.sh below)

Other:
- check-<prog>  - run set of tests against <prog> from file tests-<prog> and compare against reference results
- check         - run standard tests against each program and compare against reference results
- checkb-<prog> - run set of tests against <prog> from file tests-<prog> and compare against reference results (see check-resultsb.sh below)
- checkb        - run standard tests against each program and compare against reference results (see check-resultsb.sh below)
- results       - generate the reference results used by make check
Clean:
- clean         - clean all make outputs

7. test-vars.sh
This file contains the settings used by test-*.sh and check-*.sh
- You can modify this file to change test parameters

8. 
   tests-phashfine
   tests-phashfinerw
These files are the list of standard tests to run against each executable during make test
- if you add your own tests you can add them to these files to run them during make test


=== OTHER FILES ===

9. test-result.sh
   test-results.sh
These scripts are used by make test-* to check whether tests were successful
- This is less robust that check-result*.sh (see 10. below), but doesn't require pre-generated reference results

10. test-resultb.sh
   test-resultsb.sh
These scripts are used by make testb-* to check whether tests were successful
- Same as 10., but don't use script (which means they have more output for failing tests, but should work on MACOS or other machines with old versions of script

11. check-result.sh
   check-results.sh
These scripts validate program output by diffing against known good output
- These are more robust that test-result*.sh, so should probably be used for actual grading

12. check-resultb.sh
   check-resultsb.sh
These scripts validate program output by diffing against known good output
- Same as 11., but don't use script (which means they have more output for failing tests, but should work on MACOS or other machines with old versions of script

13. gen_results.sh
Generate the set of results used by the check-result*.sh scripts

14. README.txt
This README file

================================================


Useful precompiler macro flags
NACHOS - ignore these sections for now, this code is for proj1b
P1BTESTS - used to conditionally compile code that is project1B specific (as opposed to project 1A)

NOVALIDATE - skip validating test results (useful for performance testing, see ptest.cc)
NOYIELD - don't insert yields inserted to help uncover bugs in hashchain.cc (useful for performance testing, see hashchain.cc, make sure this is NOT defined in submitted code)

P1_RWLOCK - use the rwlock (instead of mutex)

Running:
after building the binaries (with make), you can run the ptest programs like ./phashfinerw <num threads> <num keys> <testnum or testname> <iterations>
  Ex: ./phashfinerw 200 1000 ALL_TESTS 3
  Ex: ./phashfine 200 1000 TEST_PUT 1

The ptest-simple programs can be run like ./pashcoarse-simple
  Ex: ./phashfinerw-simple
  Ex: ./phashfine-simple
  

To run through all the ptest tests for each program and just get a PASS/FAIL message you can use make test-<progname> or make test to test all programs
  Ex: make test-phashfine
  Ex: make test
  NOTE: failing tests indicate bugs, but since the race conditions are non-deterministic, passing tests don't guarantee you don't have synchronization bugs
    - try running the tests multiple times or increase the constants in test-vars.sh to improve your chances of finding any bugs
  NOTE: If you get errors relating to script not having the -e option, your machine is running an older version of script
    - there are testb and testb-* targets which don't use script. The will produce much more output when tests fail, but should work on macos or older machines
