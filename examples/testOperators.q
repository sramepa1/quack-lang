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
	
	fun main(args) {
		cout = @System.out;
		for(i = 0; i < 3; i = i+1) {
			cout->writeLine("The best number is: " + add(20, 22));
			cout->writeLine(add("Quack, ", "quack..."));
			cout->writeLine(add("Habala's constant is: ", sub(20, 7)));
			cout->writeLine(mul(42, sub(0,13))); // 42 * -13 = -546
			cout->writeLine(("666 / 42  =  " + div(666, 42)) + (", remainder " + mod(666, 42)));
		}
	}
}
