The list of important files the threads/ directory
You shouldn't modify anything outside of threads/

=== FILES TO SUBMIT ===

1. hashchain.h
This file contains class specifications for the hashmap implementations.
Need to add members for synchronization

2. hashchain.cc
This file contains HashMap implementations.
Need to add code for synchronization to the top (do not modify below the line)

3. rwlock.h
This file contains declaration for RW lock.
Need to add members for synchronization

4. rwlock.cc
This file contains definitions for RW lock
Need to add code for synchronization

5. synch.h
Synchronization headers for NACHOS
You should add any needed Lock and Condition vars here

6. synch.cc
Synchronization code for NACHOS
You should add implementations for Lock and Condition here

=== FILES TO MODIFY FOR LOCAL TESTING ===

7. threadtest-simple.cc
This is a simple test program which tests synchronized access to HashMap.
You can add additional tests here to aid in debugging

8. threadtest.cc
This is the comprehensive test program which tests synchronized access to HashMap.
You can add additional tests here to aid in debugging

9. Makefile
This is used to produce the binaries and run tests.
Comprehensive tests:
- nachos_sem    - semaphore-based synchronization
- nachos_lock   - mutex-based synchronization
- nachos_rw     - RWLock based synchronization
Simple tests:
- nachos_sem-simple    - semaphore-based synchronization
- nachos_lock-simple   - mutex-based synchronization
- nachos_rw-simple     - RWLock based synchronization
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

10. test-vars.sh
This file contains the settings used by test-*.sh and check-*.sh
- You can modify this file to change test parameters

8. tests-nachos_sem
   tests-nachos_lock
   tests-nachos_rw
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
NACHOS - used to conditionally compile code that is nachos-specific
P1BTESTS - used to conditionally compile code that is project1B specific (as opposed to project 1A)


NOVALIDATE - skip validating test results (useful for performance testing, see threadtest.cc)
NOYIELD - don't insert yields inserted to help uncover bugs in hashchain.cc (useful for performance testing, see hashchain.cc, make sure this is NOT defined in submitted code)

P1_SEMAPHORE - synchronize hashtable access with NACHOS semaphore
P1_LOCK      - synchronize hashtable access with NACHOS lock
P1_RWLOCK    - synchronize hashtable access with NACHOS-based rwlock


Running:
after building the binaries (with make), you can run the threadtest programs like ./nachos_sem -q <testnum>
  Ex: ./nachos_sem -q -1
  Ex: ./nachos_lock -q 1
  Ex: ./nachos_rw -q 12
The threadtest-simple programs can be run like ./pashcoarse-simple
  Ex: ./nachos_sem-simple
  Ex: ./nachos_lock-simple
  Ex: ./nachos_rw-simple
  

To run through all the threadtest tests for each program and just get a PASS/FAIL message you can use make test-<progname> or make test to test all programs
  Ex: make test-nachos_sem
  Ex: make test
  NOTE: failing tests indicate bugs, but since the race conditions are non-deterministic, passing tests don't guarantee you don't have synchronization bugs
    - try running the tests multiple times or increase the constants in test-vars.sh to improve your chances of finding any bugs
  NOTE: If you get errors relating to script not having the -e option, your machine is running an older version of script
    - there are testb and testb-* targets which don't use script. They will produce much more output when tests fail, but should work on macos or older machines
