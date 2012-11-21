CFLAGS = -pedantic -Wall -O0 -g3
INCLUDE = ../common/include
DEFINE = DEBUG

OBJECTS = $(patsubst src/%.cpp,build/%.o,$(wildcard src/*.cpp))

all: daisy

build:
	mkdir build

build/%.o: src/%.cpp
	g++ $(CFLAGS) -I$(INCLUDE) -D$(DEFINE) -c -o $@ $<

build/%.o: src/natives/%.cpp
	g++ $(CFLAGS) -I$(INCLUDE) -D$(DEFINE) -c -o $@ $<

daisy: build $(OBJECTS) build/SystemNative.o
	g++ $(CFLAGS) -o $@ $(OBJECTS) build/SystemNative.o

run: daisy
	./daisy ../examples/hello.qc

install: daisy
	cp daisy /usr/bin/

.PHONY: clean

clean:
	rm -rf build
	rm -f daisy

