#ifndef INTERPRETER_H
#define INTERPRETER_H

extern "C" {
	#include <stdint.h>
}
#include <vector>
#include <set>

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
	Interpreter(bool jit);
	void start(std::vector<char*>& args);
	Instruction* processInstruction(Instruction* insn);

	// GC interface
	QuaValue readRegister(uint16_t index) { return regs[index]; }
	uint16_t getMaxRegCount() { return *(methodRegCounts.rbegin()); }

private:
	std::vector<QuaValue> regs;
	std::multiset<uint16_t> methodRegCounts;

	JITCompiler* compiler;
	friend void* jitCallDirect(void* retaddr, uint64_t that, QuaSignature* sig);

	Instruction* performCall(QuaMethod* method);
	Instruction* performReturn(QuaValue retVal);
	Instruction* performThrow(QuaValue& qex);

	enum TransferReason {
		REASON_CALL = 0,
		REASON_RETURN = 1,
		REASON_THROW = 2
	};
	Instruction* transferControl(TransferReason reason, uint64_t param);

	void functionPrologue(QuaValue that, QuaMethod* method, void* retAddr, bool interpretedOrigin, uint16_t destReg);
	void functionEpilogue();
	void clearContext(int32_t regCnt);

	Instruction* directCall(uint16_t destReg, QuaValue& that, QuaSignature* sig, Instruction* retAddr);
	Instruction* commonCall(uint16_t destReg, QuaValue& that, uint16_t sigIndex, Instruction* retAddr);
	Instruction* commonException(const char* className, const char* what);

	bool taggedA3Possible(uint8_t leftTag, uint16_t rightReg);

	// Declared as returning Instruction* to allow future implementations to recover via interpreted code
	// instead of just halting the VM by throwing an exception.
	static Instruction* handleIllegalInstruction(Instruction* insn);
	static Instruction* handleIllegalSubOp(Instruction* insn);
	static Instruction* unhandledException(QuaValue qex);

	static bool isNull(QuaValue v);
	static bool parseTaggedBool(QuaValue v);

	Instruction* handleNOP(Instruction* insn);
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
	Instruction* handleNEG(Instruction* insn);
	Instruction* handleLNOT(Instruction* insn);
	Instruction* handleIDX(Instruction* insn);
	Instruction* handleIDXI(Instruction* insn);
	Instruction* handleIDXW(Instruction* insn);
	Instruction* handleIDXWI(Instruction* insn);
	Instruction* handleJMP(Instruction* insn);
	Instruction* handleCALL(Instruction* insn);
	Instruction* handleCALLMY(Instruction* insn);
	Instruction* handleNEW(Instruction* insn);
	Instruction* handleRET(Instruction* insn);
	Instruction* handleRETT(Instruction* insn);
	Instruction* handleRETNULL();
	Instruction* handleTRY(Instruction* insn);
	Instruction* handleCATCH(Instruction* insn);
	Instruction* handleTHROW(Instruction* insn);
	Instruction* handleTHROWT(Instruction* insn);
	Instruction* handleFIN(Instruction* insn);
	Instruction* handleINSTOF(Instruction* insn);
	Instruction* handleISTYPE(Instruction* insn);
	Instruction* handleCNVT(Instruction* insn);
	Instruction* handleHCF();
	Instruction* handleHLT();

	Instruction* commonIDX(Instruction* insn);
	Instruction* commonIDXW(Instruction* insn);
};

#endif // INTERPRETER_H
