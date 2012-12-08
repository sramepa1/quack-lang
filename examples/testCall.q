statclass Main {

	fun helloCaller1(stream) {
		stream->writeLine("Interpreter says hello");
	}

	// This should be JIT-compiled
	fun helloCaller2(stream, string) {
		stream->writeLineN(string); 	// test jit->native
	}
	
	// This should be JIT-compiled
	fun proxy(stream, string, test) {
		context = test;
		helloCaller1(stream);			// test jit->interpreter
		helloCaller2(stream, string);	// test jit->jit
		return context;
	}	

	fun main(args) {
		successString = "Test successful!";
		for(i = 0; i < 3; i = i+1) {
			cout = @System.out;
			test = proxy(cout, "JIT says hello!", successString);
			cout->writeLine(test);		// test variable survived OK			
		}
	}
}
