#include <iostream>

#include "globals.h"
#include "Instruction.h"
#include "Loader.h"
#include "Interpreter.h"

using namespace std;

int main(int argc, char* argv[]) {
    cout << "This is Daisy version 0.0, a Quack virtual machine." << endl;

    initGlobals();

    //loader->loadClassFile(argv[1]);
    //Interpreter interpreter;
    //try {
    //  interpreter.start(loader->getEntryPoint());
    //} catch (ExitException) {}

    cout << "Quack quack. Good bye." << endl;

	return 0;
}
