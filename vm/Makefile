CFLAGS = -pedantic -Wall -Wwrite-strings -O0 -g3
INCLUDE = ../common/include

OBJECTS = $(patsubst src/%.cpp,build/%.o,$(wildcard src/*.cpp))

all: daisy

build:
	mkdir build

build/%.o: src/%.cpp
	g++ $(CFLAGS) -I$(INCLUDE) -c -o $@ $<

daisy: build $(OBJECTS)
	g++ $(CFLAGS) -o $@ $(OBJECTS)

install: daisy
	cp daisy /usr/bin/

.PHONY: clean

clean:
	rm -rf build
	rm -f daisy

