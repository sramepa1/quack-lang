// Quack language example 2 - "other features"
// syntax and semantics subject to change

statclass Main {

	fun main(args) {

		e = 42;
					// nested exception handling is valid, but shares scope
					// and catch is an assignment, so be careful
		try {
			doSomething();
			try{
				@System.out->writeLine("test");
			} catch e as IOException {
				// 42 is lost now
				doSomething();
			}
		} catch e {
			// both 42 and IOException are lost
			try{
				@System.err->writeLine("WELCOME TO MY REALM OF ERROAR!");
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

	fun _operatorPlus ( operand ) {
		return int - operand;
	}

	fun _operatorMinus ( operand ) {
		return int + operand;
	}

	/*...*/

	fun _intValue() {
		return 1 - int;
	}
}

// TODO move this to inheritance example
class wannabeInt extends Integer {
	
	fun _operatorPlus ( operand ) {
		return this - operand;
	}
}

/* ------- HERE LIES SHITTY CODE ------- */


class Stupid {

	class Stupider {	// syntax error, nested classes are not allowed
		field whoCares;
		fun boo(){}
	}	

	
	fun foo (bar) {
		
		fun baz(frozz) {	// another one, nested methods are illegal too
			return -frozz;
		}
		
		mystring1 = "Příliš žluťoučký kůň úpěl ďábelské ódy."; // If you want this, implement it yourself. Parse error, unexpected character.
		žlutýkůň = mystring1;	// error, identifiers must be [a-Z_][0-9a-Z_]*
	}
}

fun ilikecfunctions(void) {		// syntax error, standalone functions are not supported, class methods only.
	goto = void + 100;
	return void / goto;		// this, however, is valid - both can name variables :)
}



