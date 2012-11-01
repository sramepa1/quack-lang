#include <iostream>
#include <stdexcept>
#include <csetjmp>
#include <csignal>

extern "C" {
    #include <unistd.h>
}

#include "globals.h"
#include "Instruction.h"
#include "Loader.h"
#include "Interpreter.h"

using namespace std;

bool testPointer(void* ptr, void* low, void* high, const char* message) {
    if(ptr >= low && ptr < high) {
        cerr << message << endl;
        return true;
    }
    return false;
}

void handler(int sig, siginfo_t *si, void *unused)
{
    cerr << "SIGSEGV - attempted to access protected address: " << si->si_addr << endl;
    if( !(  testPointer(si->si_addr, valStackLow, valStackHigh, "That is on the VM value stack!")
        ||  testPointer(si->si_addr, addrStackLow, addrStackHigh, "That is on the VM address stack!")
        ||  testPointer(si->si_addr, heap->getBase(), heap->getEnd(), "That is on the VM heap!"))) {
        cerr << "That is not in any mmapped region, must be a bug in the VM itself!" << endl;
    }
    siglongjmp(jmpEnv, 1);
    // wheeeeeee!
}


int main(int argc, char* argv[]) {
    cout << "This is Daisy version 0.0, a Quack virtual machine." << endl;

    // Initialize SIGSEGV catching
    struct sigaction sa;
    sa.sa_sigaction = handler;
    sa.sa_flags = SA_SIGINFO;
    sigemptyset(&sa.sa_mask);
    if(sigaction(SIGSEGV, &sa, NULL) != 0) {
        cerr << "Warning: Unable to correctly install signal handler!" << endl;
    }
    if(sigsetjmp(jmpEnv,1) == 0) {


        // This is the actual useful main() code
        try {
            initGlobals(2*getpagesize(), 2*getpagesize(), 2*getpagesize()); // super-tight for testing
            loader->loadClassFile(argv[1]);
            //interpreter.start(loader->getEntryPoint());

        } catch(runtime_error& e) {
            cerr << "Something bad has happened: " << e.what() << endl;
            return 1;

        } catch(ExitException) {
            // OK, terminate correctly
        }
        cout << "Quack quack. Good bye." << endl;
        return 0;


    } else {
        // SIGSEGV handler long jump landing:
        cerr << "QUACK OVERFLOW!" << endl;
        return 1;
    }
}
