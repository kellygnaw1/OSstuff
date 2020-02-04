# NOTE: this is a GNU Makefile.  You must use "gmake" rather than "make".
#
# Makefile for the threads assignment.  The threads assignment must
#  be done first!
#
# Copyright (c) 1992 The Regents of the University of California.
# All rights reserved.  See copyright.h for copyright notice and limitation 
# of liability and disclaimer of warranty provisions.

DEFINES = -DTHREADS
INCPATH = -I../threads -I../machine
HFILES = $(THREAD_H)
CFILES = $(THREAD_C)
C_OFILES = $(THREAD_O)

include ../Makefile.common
include ../Makefile.dep
-include deps.mk

#you can change this to get the reference results for qcheck from a different directory
#  - results,check, and check-* are unaffected by this setting
RESULTDIR = results

#test-* and test targets are used to run all the standard tests and grep the output for the success message
./PHONY: test
test: $(addprefix test-,$(PROGRAMS))
./PHONY: test-%
test-%: %
	./test-results.sh $<

#testb-* and test targets are used to run all the standard tests and grep the output for the success message without using script
./PHONY: testb
testb: $(addprefix testb-,$(PROGRAMS))
./PHONY: testb-%
testb-%: %
	./test-resultsb.sh $<

results: $(PROGRAMS)
	./gen-results.sh $^

#check* are used to verify program output against reference outputs
./PHONY: check
check: $(addprefix check-,$(PROGRAMS))
./PHONY: check-%
check-%: % results
	./check-results.sh $<

#qcheck* are exactly like check* except they don't generate reference results (so you can use this with previously generated reference outputs)
./PHONY: qcheck
qcheck: $(addprefix qcheck-,$(PROGRAMS))
./PHONY: qcheck-%
qcheck-%: %
	./check-results.sh $< $(RESULTDIR)

#checkb* are used to verify program output against reference outputs without using script
./PHONY: checkb
checkb: $(addprefix checkb-,$(PROGRAMS))
./PHONY: checkb-%
checkb-%: % results
	./check-resultsb.sh $<

#qcheckb* are exactly like checkb* except they don't generate reference results (so you can use this with previously generated reference outputs)
./PHONY: qcheckb
qcheckb: $(addprefix qcheckb-,$(PROGRAMS))
./PHONY: qcheckb-%
qcheckb-%: %
	./check-resultsb.sh $< $(RESULTDIR)

clean:
	rm -f *~ *.o swtch.s nachos_sem nachos_lock nachos_rw deps.mk results
