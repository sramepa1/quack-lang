// Quack language example
// syntax and semantics subject to change

statclass Main {

	/* TODO rethink fields in static classes - if we use "field" and mean "statfield",
		we can ditch "statfield", "statinit" and "self" at the cost of not having
		static content in non-static classes... */

	statfield hello;
	
	statinit {
		hello = "Hello";
	}

	fun Main(args) {
		System.out->writeLine(self.hello);
		try {
			var f = System.out->openFile("foo");
			do {
				var l = f->readLine();
				var a = new Adder(42);
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
		var arr = string->explode(" ");
		retutn arr[0] + arr[1] + this.toAdd;
	}
}

