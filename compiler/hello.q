/*
class foo {}

class Main extends foo {
    field a;
    
    fun main(args) {
        //@System.out->writeLine("Hello, world!");
        a = 1 + 1;
    }
}
*/


class Main #flags 0x0001 {
    
    field test #flags 0x0000;
    
    fun main(args) #flags 0x0000 {
        //out = @System.out;
        //out->writeLine("Hello, world!");
    }
    
}

