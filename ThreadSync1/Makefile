CC      = g++
CFLAGS  = -O
LDFLAGS  = -O -lpthread 

FLAGS0  = -O -lpthread -DNOLOCK
FLAGS1  = -O -lpthread 
FLAGS2  = -O -lpthread -DP1_RWLOCK

#you can change this to get the reference results for qcheck from a different directory
#  - results,check, and check-* are unaffected by this setting
RESULTDIR = results

TARGETS = phashfine phashfinerw
SIMPLE_TARGETS = phashfine-simple phashfinerw-simple

.PHONY: all
all: $(TARGETS) $(SIMPLE_TARGETS)

.PHONY: simple ptest-simple
simple ptest-simple: $(SIMPLE_TARGETS)

.PHONY: ptest
ptest: $(TARGETS)

phashchain: hashchain.cc ptest.cc
	$(CC) -o $@ $^ $(FLAGS0)

phashfine: hashchain.cc rwlock.cc ptest.cc
	$(CC) -o $@ $^ $(FLAGS1)

phashfinerw: hashchain.cc rwlock.cc ptest.cc
	$(CC) -o $@ $^ $(FLAGS2)

phashchain-simple: hashchain.cc ptest-simple.cc
	$(CC) -o $@ $^ $(FLAGS0)

phashfine-simple: hashchain.cc rwlock.cc ptest-simple.cc
	$(CC) -o $@ $^ $(FLAGS1)

phashfinerw-simple: hashchain.cc rwlock.cc ptest-simple.cc
	$(CC) -o $@ $^ $(FLAGS2)

#test-* and test targets are used to run all the standard tests and grep the output for the success message
./PHONY: test
test: $(addprefix test-,$(TARGETS))
./PHONY: test-%
test-%: %
	./test-results.sh $<

#testb-* and testb targets are used to run all the standard tests and grep the output for the success message
#  - the testb functions don't use script, since macos runs an older version of script without the -e option
./PHONY: testb
testb: $(addprefix testb-,$(TARGETS))
./PHONY: testb-%
testb-%: %
	./test-resultsb.sh $<

#results generates the output files for each standard test
#if we set RESULTDIR to something other than 'results', it also moves the result dir to that location
results: $(TARGETS)
	./gen-results.sh $^

#check* are used to verify program output against reference outputs
./PHONY: check
check: $(addprefix check-,$(TARGETS))
./PHONY: check-%
check-%: % results
	./check-results.sh $<

#qcheck* are exactly like check* except they don't generate reference results (so you can use this with previously generated reference outputs)
./PHONY: qcheck
qcheck: $(addprefix qcheck-,$(TARGETS))
./PHONY: qcheck-%
qcheck-%: %
	./check-results.sh $< $(RESULTDIR)

#check* are used to verify program output against reference outputs
./PHONY: checkb
checkb: $(addprefix checkb-,$(TARGETS))
./PHONY: checkb-%
checkb-%: % results
	./check-resultsb.sh $<

#qcheck* are exactly like check* except they don't generate reference results (so you can use this with previously generated reference outputs)
./PHONY: qcheckb
qcheckb: $(addprefix qcheckb-,$(TARGETS))
./PHONY: qcheckb-%
qcheckb-%: %
	./check-resultsb.sh $< $(RESULTDIR)

.PHONY: noyield-test
noyield-test:
	@ g++ -o $@ noyield-test.cc $(LDFLAGS)
	@ rm noyield-test

.PHONY: pthread-rwlock-test
pthread-rwlock-test:
	! grep -m1 pthread_rwlock *.cc *.h


.PHONY: clean
clean:
	rm -rf *.o phashchain $(TARGETS) $(SIMPLE_TARGETS) results
