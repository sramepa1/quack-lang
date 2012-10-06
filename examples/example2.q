// Quack language example 2 - "other features"
// syntax and semantics subject to change

statclass Main {

	field hello;

	fun Main(args) {
		
		try {
			doSomething();
			try{
				System.out->writeLine("test");
			catch e as IOException {
				doSomething();
			}

		catch e {

			try{
				System.err->writeLine("WELCOME TO MY REALM OF ERROAR!");
			catch e as IOException {
				doSomething();
			}
		}
	}

	fun doSomething() {}
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



