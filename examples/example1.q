// Quack language example
// syntax and semantics subject to change

statclass Main {

	field hello;
	
	init {
		hello = "Hello";
	}

	fun Main(args) {
		System.out->writeLine(this.hello);
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

