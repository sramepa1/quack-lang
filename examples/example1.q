// Quack language example
// syntax and semantics subject to change

/* This is also a comment */

statclass Main {

	field hello;
	
	init {
		hello = "\nHello!\n\n";		// a very small subset of C escapes will be available
	}

	fun main(args) {
		System.out->writeLine(this.hello); // voluntary this
		try {
			f = @System.out->openFile("foo");
			do {
				l = f->readLine();
				a = new Adder(42);
				a->processLine(l);
			} while(!f->eof());
		} catch e as IOException {
			@System.err->writeLine("EPIC FAIL! " + e->message());
		} catch e {
			@System.err->writeLine("fail " + e->message());
			e->printStackTrace(@System.err);
			f->close();
		}
	}
}

class Adder {
	
	field toAdd;

	init(number) {
		toAdd = number;
	}

	fun processLine(string) {
		arr = string->explode(" ");
		return arr[0] + arr[1] + toAdd;
	}
}


class subAdder extends Adder {	// valid inheritance syntax

	init(num) {		// overriden constructor
		
		g = 0;

		for(	i = 0;		/* any statement */
			i < num;	/* expression that should yield a boolValue*/
			i = i+1)	/* same as first */
		{
			g = g + i*i;
		}

		useless();		// calling a method from a constructor is allowed; all fields pre-initialized to null

		this.toAdd = -g;	// unary minus
	}

	fun useless() {
		a = b;		// allowed, both are initialized to null
		if( c == null ) { // actually compiles as "isnull(c)", special bytecode instruction, to avoid calling operator== on a null reference
			b = 3;
		}
		// a and c stay null here	
	}
}


/* ------- HERE LIES SHITTY CODE ------- */

#include "stdio.q"	// If you want directives, implement them yourself. Unexpected character '#'.

statclass Stupid extends Main {	// syntax error, static classes can't inherit anything

	field hello; 		         	// Error, redeclaration of field "hello"

	field whoCares = "Compiler does!";      // Error, initializing fieds this way is not allowed 

	foo (bar) {			        // another syntax error, fun is required ;)
		if(a = bar - 42) {	// and another one, assignment is a statement

			@System.out->writeLine("This code was executed by Chuck Norris");
		}

		for( for ( for(i=0;i<42;i=i+1) {} ; i > 42; for(j=0;j<7;i=i-1) {});false;) {
			@System.out->writeLine("This is completely terrible, but valid");
		}
	}
}




