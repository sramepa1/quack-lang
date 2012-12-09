statclass Main {
	
	fun add(a, b) {
		return a + b;
	}
	
	fun sub(a, b) {
		return a - b;
	}
	
	fun mul(a, b) {
		return a * b;
	}
	
	fun div(a, b) {
		return a / b;
	}
	
	fun mod(a, b) {
		return a % b;
	}
	
	fun lt(a, b) {
		return a < b;
	}
	
	fun le(a, b) {
		return a <= b;
	}
	
	fun gt(a, b) {
		return a > b;
	}
	
	fun ge(a, b) {
		return a >= b;
	}
	
	fun eq(a, b) {
		return a == b;
	}
	
	fun neq(a, b) {
		return a != b;
	}
	
	fun boolstr(a) {
		if(a) {
			return "true";
		} else {
			return "false";
		}
	}
	
	fun main(args) {
		cout = @System.out;
		for(i = 0; i < 3; i = i+1) {
			cout->writeLine("The answer to life is: " + add(20, 22));
			cout->writeLine(add("Quack, ", "quack..."));
			cout->writeLine(add("Habala's constant is: ", sub(20, 7)));
			cout->writeLine(mul(42, sub(0,13))); // 42 * -13 = -546
			cout->writeLine(("666 / 42  =  " + div(666, 42)) + (", remainder " + mod(666, 42)));
			
			for(j = 0; j < 3; j = j+1) {
				cout->writeLine("" + i + " < " + j + "  =  " + boolstr(lt(i,j)));
				cout->writeLine("" + i + " <= " + j + "  =  " + boolstr(le(i,j)));
				cout->writeLine("" + i + " > " + j + "  =  " + boolstr(gt(i,j)));
				cout->writeLine("" + i + " >= " + j + "  =  " + boolstr(ge(i,j)));
				cout->writeLine("" + i + " == " + j + "  =  " + boolstr(eq(i,j)));
				cout->writeLine("" + i + " != " + j + "  =  " + boolstr(neq(i,j)));
			}
		}	
	}
}
