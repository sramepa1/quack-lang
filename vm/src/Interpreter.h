#ifndef INTERPRETER_H
#define INTERPRETER_H

extern "C" {
    #include <stdint.h>
}

#include <vector>

#include "Instruction.h"
#include "JITCompiler.h"
#include "QuaValue.h"

class ExitException {};

class Interpreter
{
public:
    Interpreter();
    void start(Instruction* entryPoint);
    Instruction* processInstruction(Instruction* insn);

private:
    std::vector<QuaValue> regs;

    JITCompiler* compiler;

    static Instruction* handleIllegalInstruction(Instruction* insn);

    Instruction* handleLDC(Instruction* insn);
    Instruction* handleLDF(Instruction* insn);
    Instruction* handleLDSTAT(Instruction* insn);
};

#endif // INTERPRETER_H
