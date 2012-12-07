statclass Main {

    fun test(a) {
        b = a;
        return b;
    }

    fun main(args) {
        outstream = @System.out;
	returned = test(outstream);
        returned->writeLine("Hello, world!");
	outstream->writeLine("Goodbye, world!");
    }
}
