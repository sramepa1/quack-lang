#include "Interpreter.h"
#include <cstdlib>

using namespace std;

Interpreter::Interpreter()
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
    return NULL;
}
