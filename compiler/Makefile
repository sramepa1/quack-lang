CFLAGS = -Wall -pedantic -O0 -g

all: run

OBJECTS = $(patsubst %.cpp,build/%.o,$(wildcard ./*.cpp))

build:
	mkdir build

build/parser.cpp: parser.y
	bison -d -o $@ $^

build/parser.hpp: build/parser.cpp

build/tokens.cpp: tokens.l build/parser.hpp
	flex -o $@ $^

build/tokens.o: build/tokens.cpp
	g++ $(CFLAGS) -c -o build/tokens.o build/tokens.cpp

build/parser.o: build/parser.cpp
	g++ $(CFLAGS) -c -o build/parser.o build/parser.cpp

build/%.o: %.cpp 
	g++ $(CFLAGS) -c -o $@ $<

donald: build $(OBJECTS) build/tokens.o build/parser.o
	g++ -o $@ $(OBJECTS) build/tokens.o build/parser.o

.PHONY: clean

clean:
	rm -rf build
	rm -f donald
	

run: donald
	@echo "\n--------------------------------------------"



