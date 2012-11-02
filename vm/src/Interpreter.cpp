#include "Interpreter.h"
#include "globals.h"
#include <cstdlib>

using namespace std;

Interpreter::Interpreter() : compiler(new JITCompiler())
{
}

void Interpreter::start(Instruction* entryPoint) {
    Instruction* pc = entryPoint;
    while(1) {
        pc = processInstruction(pc);
    }
}

Instruction* Interpreter::processInstruction(Instruction* insn) {

    //TODO

    // SIGSEGV tests
    //*((char*)valStackLow + getpagesize() - 1) = 'a';
    //*((char*)addrStackLow + getpagesize() - 1) = 'a';
    //*((char*)heap->getEnd() - 1) = 'a';
    //*((char*)NULL) = 'a';

    compiler->compile(NULL);
    throw ExitException();

    //TODO

    return NULL;
}
