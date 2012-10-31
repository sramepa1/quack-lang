#ifndef INTERPRETER_H
#define INTERPRETER_H

extern "C" {
    #include <stdint.h>
}

#include <vector>

#include "Instruction.h"

class ExitException {};

class Interpreter
{
public:
    Interpreter();
    void start(Instruction* entryPoint);
    Instruction* processInstruction(Instruction* insn);

private:
    std::vector<uint64_t> regs;
};

#endif // INTERPRETER_H
