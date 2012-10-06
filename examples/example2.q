// Quack language example 2 - "other features"
// syntax and semantics subject to change

statclass Main {

	fun main(args) {

		e = 42;
		
		try {
			doSomething();
			try{					// may be valid
				System.out->writeLine("test");
			} catch e as IOException {
				// 42 is lost now
				doSomething();
			}
		} catch e {
			// both 42 and IOException are lost
			try{
				System.err->writeLine("WELCOME TO MY REALM OF ERROAR!");
			} catch e as IOException {
				// outer exception is lost now
				doSomething();
			}
		}
	}

	field a;
	field b;

	// Field shadowing example, with explicit local variable declaration:

	fun doSomething(a) : local(b, c) {	
		c = this.a + this.b;	// initialize local variable
		this.a = a/2;		// assignment to field
		a = a - 1;		// assignment to function parameter
		this.b = a + this.a;	// assignment to field
		b = 10;			// initialize local variable
		b = b + this.b + a;	// assignment to local
		this.b = b;		// assignment to field
		a = c * 10;		// redefine local
		return a;		// return local
	}
}


class pseudoInt {
	
	field int;

	fun operator+ ( operand) {
		return int - operand;
	}

	fun operator- ( operand ) {
		return int + operand;
	}

	/*...*/

	fun intValue() {
		return 1 - int;
	}
	 
}


/* ------- HERE LIES SHITTY CODE ------- */

class wannabeInt extends Integer {	// allowed or not???
	
	fun operator+ ( operand ) {
		return this - operand;
	}
}

class Stupid {

	class Stupider {	// syntax error, nested classes are not allowed
		field whoCares;
		fun boo(){}
	}	

	
	fun foo (bar) {
		
		fun baz(frozz) {	// another one, nested methods are illegal too
			return -frozz;
		}
		
		mystring1 = "Příliš žluťoučký kůň úpěl ďábelské ódy."; // OK, UTF-8 string literal
		žlutýkůň = mystring1;	// error, identifiers must be [a-Z][0-9a-Z]*

		
	}
}

fun ilikecfunctions(void) {	// syntax error, standalone functions are not supported, class methods only.
	goto = void + 100;
	return void / goto;		// this, however, is valid - both can name variables :)
}



