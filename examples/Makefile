all: hello

hello: hello.qc

hello.qc: hello.asm
	nasm -f bin -o $@ $^

.PHONY: clean

clean:
	rm -f hello.qc

