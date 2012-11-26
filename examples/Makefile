QOBJECTS = hello.qc rt.qc

all: $(QOBJECTS)
	cp rt.qc ../vm

%.qc: %.asm
	nasm -f bin -o $@ $^

.PHONY: clean

clean:
	rm -f $(QOBJECTS)
	rm -f ../vm/rt.qc

