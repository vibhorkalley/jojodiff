#
# Makefile
#
.DEFAULT: all

JDIFF_MAIN=src/main.cpp
JPATCH_MAIN=src/jpatch.cpp
SOURCES=$(patsubst src/%, %, $(wildcard src/J*.cpp ))
OBJECTS=$(addprefix bin/, $(addsuffix .o, $(basename ${SOURCES})))

all: 		jdiff jptch
full:		clean all

CPP=g++
CFLAGS=-O3 -c -funroll-loops #-Wall
LDFLAGS=-O2
DEBUG=-g -D_DEBUG
INCLUDE=-Iheaders
NATIVE=-march=native

#all:CFLAGS+=-D_FILE_OFFSET_BITS=64 -U_LARGEFILE64_SOURCE

jdiff: $(OBJECTS)
	$(CPP) $(INCLUDE) $(LDFLAGS) $(DEBUG) $(OBJECTS) -o jdiff $(JDIFF_MAIN)

jptch: $(JPATCH_MAIN)
	$(CPP) $(INCLUDE) $(LDFLAGS) $(DEBUG) -o jptch $(JPATCH_MAIN)

bin/%.o: src/%.cpp
	$(CPP) $(CFLAGS) $(INCLUDE) $(DEBUG) -o bin/$*.o src/$*.cpp

clean:
	rm -f jdiff jptch jdifd jptcd  $(OBJECTS)

