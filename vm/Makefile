CFLAGS = -pedantic -Wall -O0 -g3
INCLUDE = ../common/include src/natives src
DEFINE = DEBUG TRACE
INCDEF = $(foreach inc,$(INCLUDE),-I$(inc)) $(foreach def,$(DEFINE),-D$(def))

OBJECTS = $(patsubst src/%.cpp,build/%.o,$(wildcard src/*.cpp))
OBJECTS += $(patsubst src/natives/%.cpp,build/%.o,$(wildcard src/natives/*.cpp))

all: daisy

build:
	mkdir build

build/%.o: src/%.cpp
	g++ $(CFLAGS) $(INCDEF) -c -o $@ $<

build/%.o: src/natives/%.cpp
	g++ $(CFLAGS) $(INCDEF) -c -o $@ $<

daisy: build $(OBJECTS)
	g++ $(CFLAGS) -o $@ $(OBJECTS)

run: daisy
	./daisy ../examples/hello.qc

install: daisy
	cp daisy /usr/bin/

.PHONY: clean

clean:
	rm -rf build
	rm -f daisy

