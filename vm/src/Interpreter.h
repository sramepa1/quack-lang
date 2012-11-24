#ifndef INTERPRETER_H
#define INTERPRETER_H

extern "C" {
    #include <stdint.h>
}
#include <vector>

#include "Instruction.h"
#include "JITCompiler.h"
#include "QuaValue.h"
#include "QuaMethod.h"


class ExitException {};

class Interpreter
{
public:
    Interpreter();
    void start(uint16_t mainClassType);
    Instruction* processInstruction(Instruction* insn);

private:
    std::vector<QuaValue> regs;

    JITCompiler* compiler;

    Instruction* performCall(QuaMethod* method);
    Instruction* performReturn(QuaValue retVal);

    static Instruction* handleIllegalInstruction(Instruction* insn);

    Instruction* handleLDC(Instruction* insn);
    Instruction* handleLDF(Instruction* insn);
    Instruction* handleLDSTAT(Instruction* insn);

    Instruction* handlePUSHC(Instruction* insn);

    Instruction* handleCALL(Instruction* insn);
};

#endif // INTERPRETER_H
