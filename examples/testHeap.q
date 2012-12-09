class Load {

    init() {}
    
    field a;
    field b;
    field c;
    field d;
    field e;
    field f;
    field g;
    field h;
    field i;
    field j;
    field k;
    field l;
    field m;
    field n;
    field o;
    field p;
    field q;
    field r;
    field s;
    field t;
    field u;
    field v;
    field w;
    field x;
    field y;
    field z;
} 


statclass Main {
    fun main(args) {
        outstream = @System.out;
        outstream->writeLine("GC smoke test!");
        
        while(true) {
            var1 = new Load();
        }
        
    }
}

