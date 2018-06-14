# NOTE: this is a GNU Makefile.  You must use "gmake" rather than "make".
#
# Makefile for the threads assignment.  The threads assignment must
#  be done first!
#
# Copyright (c) 1992 The Regents of the University of California.
# All rights reserved.  See copyright.h for copyright notice and limitation 
# of liability and disclaimer of warranty provisions.

# Depending on your platform, you need to select the correct definition.
# Also, you need to edit the Makefile in the bin subdirectory.

# DEC MIPS, Ultrix
#HOST = -DHOST_MIPS

# SUN SPARC, Sun 4.xx 
# HOST = -DHOST_SPARC -DHOST_IS_BIG_ENDIAN
# LDFLAGS = -lsocket -lnls

# HP PA-RISC, HP_UX
# HOST = -DHOST_SNAKE -DHOST_IS_BIG_ENDIAN

# 386, 386BSD Unix, or NetBSD Unix (available via anon ftp 
#    from agate.berkeley.edu)
# also, Linux
# HOST = $(CSE120_HOST)
# LDFLAGS = $(CSE120_LDFLAGS)

# slight variant for 386 FreeBSD
# HOST = -DHOST_i386 -DFreeBSD
# CPP=/usr/bin/cpp

HOST = -DHOST_i386



INCPATH =

#This is a minimal Makefile for compiling cs170 project 1b (as of spring 2017)

# You might want to play with the CFLAGS, but if you use -O it may
# break the thread system.  You might want to use -fno-inline if
# you need to call some inline functions from the debugger.

# Copyright (c) 1992 The Regents of the University of California.
# All rights reserved.  See copyright.h for copyright notice and limitation 
# of liability and disclaimer of warranty provisions.

# -fwritable-strings: deprecated
CFLAGS = -g -Wall -Wshadow $(INCPATH) $(DEFINES) $(HOST) -DCHANGED -Wno-write-strings

# These definitions may change as the software is updated.
# Some of them are also system dependent
CPP= gcc -E
CC = g++ --std=c++03 -m32
LD = g++ --std=c++03 -m32
AS = as --32
MAKE = make

PROGRAM1 = nachos_sem
PROGRAM2 = nachos_lock
PROGRAM4 = nachos_rw
SYNCHFLAGS = -DP1_RWLOCK
PROGRAMS = $(PROGRAM1) $(PROGRAM2) $(PROGRAM4)
SIMPLE_PROGRAMS = $(PROGRAM1)-simple $(PROGRAM2)-simple $(PROGRAM4)-simple

#extra flags needed for compile proj1b code
PROGRAM_FLAGS =-DNACHOS -DCHANGED -DTHREADS -DP1BTESTS $(INCPATH)


THREAD_H =copyright.h\
	list.h\
	scheduler.h\
	synch.h \
	synchlist.h\
	system.h\
	thread.h\
	utility.h\
	interrupt.h\
	sysdep.h\
	stats.h\
	timer.h\
	hashchain.h\
	rwlock.h

THREAD_C =main.cc\
	list.cc\
	scheduler.cc\
	synch.cc \
	synchlist.cc\
	system.cc\
	thread.cc\
	utility.cc\
	threadtest.cc\
	interrupt.cc\
	sysdep.cc\
	stats.cc\
	timer.cc\
	hashchain.cc\
	rwlock.cc

THREAD_S = switch.s

THREAD_O =main.o list.o scheduler.o synch.o synchlist.o system.o thread.o \
	utility.o interrupt.o stats.o sysdep.o timer.o

S_OFILES = switch.o

OFILES = $(C_OFILES) $(S_OFILES)

DEFINES = -DTHREADS
HFILES = $(THREAD_H)
CFILES = $(THREAD_C)
C_OFILES = $(THREAD_O)

.PHONY: all ptest ptest-simple simple
all:
	@ $(MAKE) depend 
	@ $(MAKE) $(PROGRAM1) 
	@ $(MAKE) $(PROGRAM2)
	@ $(MAKE) $(PROGRAM4)
	@ $(MAKE) $(PROGRAM1)-simple
	@ $(MAKE) $(PROGRAM2)-simple
	@ $(MAKE) $(PROGRAM4)-simple
ptest:
	@ $(MAKE) depend 
	@ $(MAKE) $(PROGRAM1) 
	@ $(MAKE) $(PROGRAM2)
	@ $(MAKE) $(PROGRAM4)

simple ptest-simple:
	@ $(MAKE) depend 
	@ $(MAKE) $(PROGRAM1) 
	@ $(MAKE) $(PROGRAM2)
	@ $(MAKE) $(PROGRAM4)

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

$(PROGRAM1): SYNCHFLAGS=-DP1_SEMAPHORE
$(PROGRAM2): SYNCHFLAGS=-DP1_LOCK
$(PROGRAM4): SYNCHFLAGS=-DP1_RWLOCK

$(PROGRAM1)-simple: SYNCHFLAGS=-DP1_SEMAPHORE
$(PROGRAM2)-simple: SYNCHFLAGS=-DP1_LOCK
$(PROGRAM4)-simple: SYNCHFLAGS=-DP1_RWLOCK

#NOTE: we directly compile the cc files for rwlock,hashchain,threadtest so that the flags get included properly at compile time
$(PROGRAMS): $(OFILES) rwlock.cc hashchain.cc threadtest.cc $(THREAD_H)
	@ echo $@
	@ $(LD) $(OFILES) rwlock.cc hashchain.cc threadtest.cc $(PROGRAM_FLAGS) $(LDFLAGS) $(SYNCHFLAGS) $(TESTFLAGS) -o $@ -g

#NOTE: we directly compile the cc files for rwlock,hashchain,threadtest so that the flags get included properly at compile time
$(SIMPLE_PROGRAMS): $(OFILES) rwlock.cc hashchain.cc threadtest-simple.cc $(THREAD_H)
	@ echo $@
	@ $(LD) $(OFILES) rwlock.cc hashchain.cc threadtest-simple.cc $(PROGRAM_FLAGS) $(LDFLAGS) $(SYNCHFLAGS) $(TESTFLAGS) -o $@ -g


$(C_OFILES): %.o:
	@ echo $<
	@ $(CC) $(CFLAGS) -c $< -g

switch.o: switch.s
	@ echo $<
	@ $(CPP) -P -x c $(INCPATH) $(HOST) switch.s > swtch.s
	@ $(AS) -o switch.o swtch.s

depend: $(CFILES) $(HFILES)
	@ echo building dependencies
	@ $(CC) $(INCPATH) $(DEFINES) $(HOST) -DCHANGED -DNACHOS -DP1BTESTS -M $(CFILES) > deps.mk

-include deps.mk

.PHONY:clean
clean:
	rm -rf *.o *~ nachos_sem nachos_lock nachos_rw deps.mk results
