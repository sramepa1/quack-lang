#ifndef JITCOMPILER_H
#define JITCOMPILER_H

#include <vector>
#include <list>
#include <map>
#include <climits>

extern "C" {
#include "stdint.h"
}

#include "Instruction.h"
#include "QuaMethod.h"

class Interpreter;

class GiveUpException {};

class JITCompiler
{
public:
	JITCompiler(bool enabled); // Defined in interpreter.cpp

	bool compile(QuaMethod* method);

private:

	bool enabled;

	static void* ptrToVMBP;
	static void* ptrToVMSP;

	static void* callLabel;
	static void* returnLabel;
	static void* throwLabel;

	static void* whatLabel;

	friend void* jitCallDirect(void* retaddr, uint64_t that, QuaSignature* sig);

	// in encoding order (bit 3 in enum = REX required)
	enum MachineRegister {
		REG_RAX = 0x0,
		REG_RCX = 0x1,
		REG_RDX = 0x2,
		REG_RBX = 0x3,
		REG_RSP = 0x4,
		REG_RBP = 0x5,
		REG_RSI = 0x6,
		REG_RDI = 0x7,

		REG_R8 = 0x8,
		REG_R9 = 0x9,
		REG_R10 = 0xA,
		REG_R11 = 0xB,
		REG_R12 = 0xC,
		REG_R13 = 0xD,
		REG_R14 = 0xE,
		REG_R15 = 0xF
	};

	std::list<Instruction*> buildObjects(QuaMethod* method);

	void updateMax(uint16_t& maxReg, uint16_t insnReg);
	std::map<uint16_t, MachineRegister> allocateRegisters(std::list<Instruction*> insns);
	void generate(std::list<Instruction*> insns, std::map<uint16_t, MachineRegister> allocation,
				  std::vector<unsigned char>& buffer);

	// Code generation

	void append(std::vector<unsigned char>& buffer, const char* data, size_t count);

	void emitOneByteInsn(MachineRegister reg, unsigned char machineOp, std::vector<unsigned char>& buffer,
						 bool longMode = true);

	void emitModRMInsn(MachineRegister regRM, MachineRegister regR, const char *opcode, int opLength,
						std::vector<unsigned char>& buffer, bool directAddressing,
						int32_t displacement = 0, bool longMode = true);

	void emitPrepareContext(std::map<uint16_t, MachineRegister> allocation, std::vector<unsigned char>& buffer);
	void emitContextOperation(bool save, std::map<uint16_t, MachineRegister> allocation,
							  std::vector<unsigned char>& buffer);
	void emitSwitchPointersToC(std::vector<unsigned char>& buffer);
	void emitSwitchPointersToJIT(std::vector<unsigned char>& buffer);
	void emitLoadLabelToRax(void* labelPtr, std::vector<unsigned char>& buffer);
	void emitJumpToLabel(void* labelPtr, std::vector<unsigned char>& buffer);

	typedef union {
		uint16_t sigIndex;
		QuaSignature* sig;
	} CallDestination;
	void translateCall(std::map<uint16_t, MachineRegister> allocation, std::vector<unsigned char>& buffer,
						CallDestination destination, bool directCall,
						MachineRegister thatMoveRegRM, unsigned char thatMoveOpcode,
						bool thatMoveDirectAddressing, int32_t thatMoveDisplacement);

	void translateA3REG(std::map<uint16_t, MachineRegister> allocation, std::vector<unsigned char>& buffer,
					unsigned char quackSop, MachineRegister destReg, MachineRegister leftReg, MachineRegister rightReg);

	void translateLNOT(std::map<uint16_t, MachineRegister> allocation, std::vector<unsigned char>& buffer,
							MachineRegister destReg, MachineRegister srcReg);

	void translateISTYPE(std::vector<unsigned char>& buffer,
							MachineRegister destReg, MachineRegister srcReg, uint16_t type);

	void translateCNVT(std::map<uint16_t, MachineRegister> allocation, std::vector<unsigned char>& buffer,
									unsigned char quackSop, MachineRegister destReg, MachineRegister srcReg);

	void translateStackOp(Instruction* insn, unsigned char opcode,
						std::map<uint16_t, MachineRegister> allocation, std::vector<unsigned char>& buffer);

	void translateFieldAccess(std::map<uint16_t, MachineRegister> allocation, std::vector<unsigned char>& buffer,
						unsigned char quackOp, uint16_t imm16, MachineRegister regDestSrc, MachineRegister regThat);

	void setupLeave(std::vector<unsigned char>& buffer);
	void finishLeaveReg(MachineRegister reg, void* destinationLabel, std::vector<unsigned char>& buffer);
};

#endif // JITCOMPILER_H
