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


gentest:
	@echo "End the process when you wish, depending on the desired file size"
	cat /dev/urandom | tr -dc a-zA-Z0-9 > $(FILE_NAME)

# OUT_FILE = desired output file name (patch name)
runtest:
	time ./$(DIFF_EXE) -m 0 $(TEST1) $(TEST2) > $(OUT_FILE)
	@echo
	@echo "Patching..."
	./$(PTCH_EXE) $(TEST1) $(OUT_FILE) > patched_version
	@echo
	@echo "Verifying desired and resulted file:"
	md5sum $(TEST2) patched_version
	@echo
clean:
	rm -f $(DIFF_EXE) $(PTCH_EXE) $(OBJECTS) $(OUT_FILE) patched_version

.DEFAULT:	all
.PHONY:		clean
