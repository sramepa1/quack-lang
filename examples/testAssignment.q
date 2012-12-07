statclass Main {

    fun test(a) {
        b = a;
        return b;
    }

    fun main(args) {
        outstream = @System.out;
	returned1 = test(outstream);
	returned2 = test(returned1);
	returned3 = test(returned2);
        returned3->writeLine("Hello, world!");
	outstream->writeLine("Goodbye, world!");
    }
}
