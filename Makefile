#
# Makefile
#
JDIFF_MAIN=src/main.cpp
JPATCH_MAIN=src/jpatch.cpp
SOURCES=$(patsubst src/%, %, $(wildcard src/J*.cpp ))
OBJECTS=$(addprefix bin/, $(addsuffix .o, $(basename ${SOURCES})))

SHELL := /bin/bash
DIFF_EXE=jdiff
PTCH_EXE=jptch

all:	  $(DIFF_EXE) $(PTCH_EXE)
debug:	  all
full:	  clean all
parallel: debug

CPP=g++
WARNINGS=-Wno-cpp# -Wno-format -Wall
INCLUDE=-Iheaders

CFLAGS=-c# -O3 -funroll-loops
CFLAGS+=$(WARNINGS)
CFLAGS+=-D_FILE_OFFSET_BITS=64 -U_LARGEFILE64_SOURCE

LDFLAGS=#-O2
LDFLAGS+=$(WARNINGS)

debug:DEBUG=-g -D_DEBUG
parallel:CFLAGS+=-fopenmp -lpthread
parallel:LDFLAGS+=-fopenmp -lpthread

$(DIFF_EXE): $(OBJECTS)
	$(CPP) $(INCLUDE) $(LDFLAGS) $(DEBUG) $(OBJECTS) $(JDIFF_MAIN) -o $@

$(PTCH_EXE): $(JPATCH_MAIN)
	$(CPP) $(INCLUDE) $(LDFLAGS) $(DEBUG) $(JPATCH_MAIN) -o $@

bin/%.o: src/%.cpp
	$(CPP) $(INCLUDE) $(CFLAGS) $(DEBUG) -o bin/$*.o src/$*.cpp

# for generate test file: make gentest SIZE=100
gentest:
	dd if=/dev/urandom of=tests/$(SIZE)MB bs=1MB count=$(SIZE)

# for running a test: make runtest TEST=path/to/testfile
runtest:
	time ./$(DIFF_EXE) $(TEST) $(TEST) &> /dev/null

clean:
	rm -f $(DIFF_EXE) $(PTCH_EXE) $(OBJECTS)

.DEFAULT:	all
.PHONY:		clean
