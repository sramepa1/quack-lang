// Quack language example
// syntax and semantics subject to change

/* This is also a comment */

statclass Main {

	field hello;
	
	init {
		hello = "Hello";
	}

	fun Main(args) {
		System.out->writeLine(this.hello);
		try {
			f = System.out->openFile("foo");
			do {
				l = f->readLine();
				a = new Adder(42);
				a->processLine(l);
			} while(!f->eof());
		} catch e as IOException {
			System.err->writeLine("EPIC FAIL! " + e->message());
		} catch e {
			System.err->writeLine("fail " + e->message());
			e->printStackTrace(System.err);
			f->close();
		}
	}
}

class Adder {
	
	field toAdd;

	init(number) {
		this.toAdd = number;
	}

	fun processLine(string) {
		arr = string->explode(" ");
		retutn arr[0] + arr[1] + this.toAdd;
	}
}

class subAdder extends Adder {

	field toAdd; 		// TODO what to do with this? Redeclaration is not altering the field table at all...

	init(num) {		// overriden constructor
		
		g = 0;

		for(	i = 0;		/* any statement */
			i < num;	/* expression that should yield a boolValue*/
			i = i+1)	/* same as first */
		{
			g = g + i*i
		}

		useless()		// allowed; all fields pre-initialized to null

		this.toAdd = -g;	// unary minus
	}

	useless() {
		a = b;		// allowed, both are initialized to null
		if( c == null ) { // actually compiles as "isnull(c)", special bytecode instruction, to avoid calling operator== on a null reference
			b = 3;
		}
		// a and c stay null here
	}
}


/* ------- HERE LIES SHITTY CODE ------- */

statclass Stupid extends Main {	// syntax error, static classes can't inherit anything
	field whoCares;

	foo (bar) {			// another syntax error, fun is required ;)
		if(a = bar - 42) {	// and another one, assignment is a statement

			System.out->writeLine("This code was executed by Chuck Norris");
		}

		for( for ( for(i=0;i<42;i=i+1) {} ; i > 42; for(j=0;j<7;i=i-1) {});false;) {
			System.out->writeLine("This is completely terrible, but valid");
		}
	}
}




