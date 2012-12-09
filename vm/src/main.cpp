#include <iostream>
#include <vector>
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
		||  testPointer(si->si_addr, heap->getVolatileBase(), heap->getVolatileEnd(), "That is on the VM volatile heap!")
                ||  testPointer(si->si_addr, heap->getPermanentBase(), heap->getPermanentEnd(), "That is on the VM permanent heap!"))) {
		cerr << "That is not in any mmapped region, must be a bug in the VM itself!" << endl;
	}
	siglongjmp(jmpEnv, 1);
	// wheeeeeee!
}

void initSigsegv() {
	struct sigaction sa;
	sa.sa_sigaction = handler;
	sa.sa_flags = SA_SIGINFO;
	sigemptyset(&sa.sa_mask);
	if(sigaction(SIGSEGV, &sa, NULL) != 0) {
		cerr << "Warning: Unable to correctly install signal handler!" << endl;
	}
}



int main(int argc, char* argv[]) {

#ifdef DEBUG
	cout << "This is Daisy version " << VM_VERSION << ", a Quack virtual machine." << endl;
#endif

	try {

		if(argc < 2) {
			throw invalid_argument("Not enough command line arguments.");
		}

		if(sigsetjmp(jmpEnv,1) == 0) {
			initSigsegv();

			int i;
			bool jit = true;
			if(strcmp(argv[1],"-nojit") == 0) {
				jit = false;
				i = 2;
			} else {
				i = 1;
			}
			if(i == argc) {
				throw invalid_argument("Expected class file name.");
			}

               initGlobals(jit, 2*getpagesize(), 2*getpagesize(), 1000*getpagesize(), 20*getpagesize()); // super-tight for testing
			for( ; i < argc && strcmp(argv[i], "-args") != 0; i++) {
				loader->loadClassFile(argv[i]);	// TODO: Are we fully ready for this? What about cross-CF inheritance?
			}
			vector<char*> args;
			for(i += 1; i < argc; i++) {
				args.push_back(argv[i]);
			}
			interpreter->start(args);
		} else {
			// SIGSEGV handler long jump landing
			throw runtime_error("QUACK OVERFLOW!");
		}

	} catch(invalid_argument& e) {
		cerr << e.what() << endl;
		cerr << "Usage: " << argv[0] << " [-nojit] <classfile> [<classfile> ...] [-args <arg1> <arg2> ...]" << endl;
		return 2;

	} catch(runtime_error& e) {
		cerr << "Something bad has happened:" << endl << e.what() << endl;
		return 1;

	} catch(ExitException) {
		// OK, terminate correctly
	}

#ifdef DEBUG
	cout << "Quack quack. Good bye." << endl;
#endif
	return 0;
}
