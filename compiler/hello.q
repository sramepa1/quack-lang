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

class foo{}

class Main #flags 0xCCCC {
    
    field test #flags 0xAAAA;
    
    field test2 #flags 0xAAAA;
    
    fun main(args) #flags 0xBBBB {
        blah = @FILE.f;
        out = @System.out;
        out->writeLine("Hello, world!");
    }
    
}

