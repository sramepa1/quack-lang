#ifndef INTERPRETER_H
#define INTERPRETER_H

extern "C" {
    #include <stdint.h>
}
#include <vector>

#include "Instruction.h"
#include "JITCompiler.h"
#include "QuaClass.h"
#include "QuaValue.h"
#include "QuaMethod.h"
#include "QuaSignature.h"


class ExitException {};

class Interpreter
{
public:
    Interpreter();
    void start();
    Instruction* processInstruction(Instruction* insn);

private:
    std::vector<QuaValue> regs;

    JITCompiler* compiler;

    Instruction* performCall(QuaClass* type, QuaSignature* sig);
    Instruction* performReturn(QuaValue retVal);

    static Instruction* handleIllegalInstruction(Instruction* insn);

    Instruction* handleLDC(Instruction* insn);
    Instruction* handleLDF(Instruction* insn);
    Instruction* handleLDSTAT(Instruction* insn);
    Instruction* handleLDCT(Instruction* insn);
    Instruction* handleLDNULL(Instruction* insn);
    Instruction* handleSTF(Instruction* insn);
    Instruction* handleLDMYF(Instruction* insn);
    Instruction* handleSTMYF(Instruction* insn);
    Instruction* handleMOV(Instruction* insn);
    Instruction* handleXCHG(Instruction* insn);
    Instruction* handlePUSH(Instruction* insn);
    Instruction* handlePOP(Instruction* insn);
    Instruction* handlePUSHC(Instruction* insn);
    Instruction* handlePUSHCT(Instruction* insn);
    Instruction* handleLDS(Instruction* insn);
    Instruction* handleSTS(Instruction* insn);
    Instruction* handleA3REG(Instruction* insn);
    Instruction* handleAREGIMM(Instruction* insn);
    Instruction* handleNEG(Instruction* insn);
    Instruction* handleLNOT(Instruction* insn);
    Instruction* handleIDX(Instruction* insn);
    Instruction* handleIDXI(Instruction* insn);
    Instruction* handleJMP(Instruction* insn);
    Instruction* handleJCC(Instruction* insn);
    Instruction* handleCALL(Instruction* insn);
    Instruction* handleCALLMY(Instruction* insn);
    Instruction* handleNEW(Instruction* insn);
    Instruction* handleRET(Instruction* insn);
    Instruction* handleRETT(Instruction* insn);
    Instruction* handleTRY(Instruction* insn);
    Instruction* handleCATCH(Instruction* insn);
    Instruction* handleTHROW(Instruction* insn);
    Instruction* handleTHROWT(Instruction* insn);
    Instruction* handleFIN(Instruction* insn);
};

#endif // INTERPRETER_H
