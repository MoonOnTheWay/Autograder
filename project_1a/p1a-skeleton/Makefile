CC      = g++
CFLAGS  = -O
LDFLAGS  = -O -lpthread 

FLAGS0  = -O -lpthread -DNOLOCK
FLAGS1  = -O -lpthread
FLAGS2  = -O -lpthread -DP1_RWLOCK
FLAGS3  = -O -lpthread -DFINEGRAIN
FLAGS4  = -O -lpthread -DFINEGRAIN -DP1_RWLOCK

#you can change this to get the reference results for qcheck from a different directory
#  - results,check, and check-* are unaffected by this setting
RESULTDIR = results

#primary test programs (gradescope tests will be performed against these with the default ptest.cc)
# - you can modify ptest.cc to add extra local tests (then also add the tests to the files tests-* for each program that should use them)
TARGETS = phashcoarse phashcoarserw  phashfine phashfinerw
#simple test programs and code examples (you can modify ptest-simple.cc to run extra local tests)
SIMPLE_TARGETS = phashcoarse-simple phashcoarserw-simple phashfine-simple phashfinerw-simple

.PHONY: all
all: $(TARGETS) $(SIMPLE_TARGETS)

phashchain: hashchain.cc ptest.cc
	$(CC) -o $@ $^ $(FLAGS0)

phashcoarse: hashchain.cc rwlock.cc ptest.cc
	$(CC) -o $@ $^ $(FLAGS1)

phashcoarserw: hashchain.cc rwlock.cc ptest.cc
	$(CC) -o $@ $^ $(FLAGS2)

phashfine: hashchain.cc rwlock.cc ptest.cc
	$(CC) -o $@ $^ $(FLAGS3)

phashfinerw: hashchain.cc rwlock.cc ptest.cc
	$(CC) -o $@ $^ $(FLAGS4)

phashchain-simple: hashchain.cc ptest-simple.cc
	$(CC) -o $@ $^ $(FLAGS0)

phashcoarse-simple: hashchain.cc rwlock.cc ptest-simple.cc
	$(CC) -o $@ $^ $(FLAGS1)

phashcoarserw-simple: hashchain.cc rwlock.cc ptest-simple.cc
	$(CC) -o $@ $^ $(FLAGS2)

phashfine-simple: hashchain.cc rwlock.cc ptest-simple.cc
	$(CC) -o $@ $^ $(FLAGS3)

phashfinerw-simple: hashchain.cc rwlock.cc ptest-simple.cc
	$(CC) -o $@ $^ $(FLAGS4)

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


.PHONY: clean
clean:
	rm -rf *.o phashchain $(TARGETS) $(SIMPLE_TARGETS) results
