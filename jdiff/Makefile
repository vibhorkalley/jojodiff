CC=g++
CFLAGS=-O3 -c -Wall -g -funroll-loops
LDFLAGS=-O2 -lm
INCLUDE=-Iinclude

SOURCES=$(patsubst src/%, %, $(wildcard src/*.cpp))
OBJECTS=$(addprefix bin/, $(addsuffix .o, $(basename ${SOURCES})))

EXECUTABLE=jdiff

all: $(OBJECTS) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

bin/%.o: src/%.cpp
	$(CC) $(CFLAGS) $(INCLUDE) -o bin/$*.o src/$*.cpp

clean:
	-rm -f ${EXECUTABLE} ${OBJECTS}

run:
	./$(EXECUTABLE)

.PHONY: all clean

