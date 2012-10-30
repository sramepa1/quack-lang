// Quack language example 3 - "inheritance"
// syntax and semantics subject to change


statclass Main {

        fun main(args) : local(good, better, best) {

                try {                
        
                        good = new GoodClass();
                        better = new BetterClass(159);
                        best = new BestClass(1, 2, 3);

                        good->uselessMethod();                  // calls uselessMethod() from GoodClass
                        better->uselessMethod();                // calls uselessMethod() from GoodClass
                        best->uselessMethod();                  // calls uselessMethod() from BestClass

                        good->someUsefulMethod(5);              // calls someUsefulMethod(param) from GoodClass
                        better->someUsefulMethod(5);            // calls someUsefulMethod(foo) from BetterClass
                        best->someUsefulMethod(5);              // calls someUsefulMethod(foo) from BetterClass

                        // calls someUsefulMethod(many, different, params) from BetterClass
                        better->someUsefulMethod(99, 88, 77);   
                        
                        // calls someUsefulMethod(many, different, params) from BetterClass
                        best->someUsefulMethod(99, 88, 77);
                        
                        // throws NoSuchMethodException
                        good->someUsefulMethod(99, 88, 77);
                        
                } catch e as NoSuchMethodException {
                        
                }
        }
}

class GoodClass {

        field one;
        filed two;

        // Constructor with two params
        init(first, second) {
                this.one = first;
                two = second;
        }

        // Default constructor
        init() {
                one = 45;
                two = 64;
        }


        // Some methods
        fun someUsefulMethod(param) {
                output = param + 42 + this.one;
                return output;
        }

        fun uselessMethod() {
                return 41;
        }

}

class BetterClass extends GoodClass {
        
        // No super class construtor specified, so it is called the default one (init() from GoodClass)
        init(a, b) {
                one = a + b + 25;
                // field two contains 64 
        }

        init(someValue) : super(someValue, 72) {
                @System.out->writeInt(one);    // prints the _intValue of param someValue
                @System.out->writeInt(two);    // prints the number 72
        }

        // Overrided someUsefulMethod(param) from GoodClass, no matter that its param has different name
        fun someUsefulMethod(foo) {
                
                localVar = super->someUsefulMethod(foo + 13);    // calls someUsefulMethod(param) from GoodClass
                one = uselessMethod();                           // field one now contains 41
                
        }

        // This method does not override any method from GoodClass
        fun someUsefulMethod(many, different, params) {
                someUsefulMethod(many - different * params);            // calls someUsefulMethod(foo) from BetterClass
                super->someUsefulMethod(many - different * params);     // calls someUsefulMethod(param) from GoodClass
        }

}

class BestClass extends BetterClass {
        
        field three;

        // Every constructor of this class has to specify which constructor from super class has to be called
        // because BetterClass has no default contructor
        init(abc, def, ghi) : super(abc) {
                three = def / ghi;
        }

        // Overrided uselessMethod() from GoodClass, because this method is not overriden in BetterClass
        fun uselessMethod() {
                super->uselessMethod();                 // calls uselessMethod() from GoodClass
                someUsefulMethod(three);                // calls someUsefulMethod(foo) from BetterClass
                super->someUsefulMethod(three);         // calls someUsefulMethod(foo) from BetterClass, too
        }
        
}


/* ------- HERE LIES SHITTY CODE ------- */

// Multiple inheritance is not allowed!
class BadClass extends String, GoodClass {

}

class UglyClass extends BetterClass {
        fun uglyMethod() {
                // The method someUsefulMethod(param) from GoodClass cannot be accesed from this class
                // because it is overriden in direct super class
                super->super->someUsefulMethod(11);
        }
}

