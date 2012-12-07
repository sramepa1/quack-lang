statclass Main {
    fun main(args) {
        outstream = @System.out;
        for(i = 0; i < 5; i = i + 1) {
            outstream->writeLineN("Good bye, cruel world!");
            this->methodForJIT();
        }
    }

    fun methodForJIT() {
        // Nothing here
    }
}