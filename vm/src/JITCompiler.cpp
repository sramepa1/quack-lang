#include "JITCompiler.h"
#include "bytecode.h"
#include "globals.h"
#include "helpers.h"
#include "runtime.h"

#ifdef DEBUG
#include <iostream>
#include <cerrno>
#elif defined TRACE
#include <iostream>
#endif

#include <cstring>

extern "C" {
	#include <unistd.h>
	#include <sys/mman.h>
}

using namespace std;

void* JITCompiler::callLabel;
void* JITCompiler::returnLabel;
void* JITCompiler::throwLabel;
void* JITCompiler::whatLabel;

void* JITCompiler::ptrToASP;
void* JITCompiler::ptrToVMBP;
void* JITCompiler::ptrToVMSP;

// Constructor defined in Interpreter.cpp


#define MACHINE_MAX_REG 11
// Means allocating to 12 registers.
// RCX, RDX, RSI, RDI, R8 ~ R15


bool JITCompiler::compile(QuaMethod* method) {

	if(!enabled) {
		#ifdef TRACE
		cout << "JIT is disabled, ";
		#endif
		return false;
	}

	#ifdef DEBUG
		if(method->action != QuaMethod::COMPILE) {
			cerr << "Warning: Attempted to compile a method with action=" << method->action << " !" << endl;
		}
	#endif

	vector<unsigned char> buffer;
	buffer.reserve(getpagesize());
	void* memblob;

	try {
		list<Instruction*> insns = buildObjects(method); //signature may change
		map<uint16_t, MachineRegister> allocation = allocateRegisters(insns);
		generate(insns, allocation, buffer);

		size_t blobsize = ((buffer.size() - 1) / getpagesize() + 1) * getpagesize();
		memblob = mmap(NULL, blobsize, PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
		if(memblob == MAP_FAILED) {
			#ifdef DEBUG
				cerr << "Warning: JIT mmap failed with error: " << strerror(errno) << endl;
			#endif
			throw GiveUpException();
		}
		#ifdef DEBUG
			// preventive INT3s
			memset(memblob, 0xCC, blobsize);
		#endif
		memcpy(memblob, buffer.data(), buffer.size());
		if(mprotect(memblob, blobsize, PROT_READ | PROT_EXEC) != 0) {
			#ifdef DEBUG
				cerr << "Warning: JIT mprotect failed with error: " << strerror(errno) << endl;
			#endif
			throw GiveUpException();
		}

	} catch(GiveUpException) {
		return false;
	}

	method->code = memblob;
	return true;
}


list<Instruction*> JITCompiler::buildObjects(QuaMethod* method) {

	list<Instruction*> ilist;

	// Due to lack of time, this JIT compiler is very simple and limited.
	// A more complex compiler might need to insert/remove/rearrange instructions (hence the linked list)
	//   and would probably make it a list of something else, probably an object representation of x86_64.

	// Consider this method a placeholder that also manages to somehow fuel a working prototype.

	for(Instruction* iptr = (Instruction*)method->code; iptr < (Instruction*)method->code + method->insnCount; iptr++) {
		ilist.push_back(iptr);
	}

	return ilist;
}


inline void JITCompiler::updateMax(uint16_t& maxReg, uint16_t insnReg) {
	if(insnReg > maxReg && insnReg != REG_DEV_NULL) {
		maxReg = insnReg;
	}
}

map<uint16_t, JITCompiler::MachineRegister> JITCompiler::allocateRegisters(list<Instruction*> insns) {

	/*
	 *	This was originally planned to house a Linear Scan register allocation mechanism -
	 *	first a data flow/liveness analysis pass and then mapping virtual registers
	 *	to machine ones or stack locations.
	 *
	 *	Unfortunately, even though I have already implemented the algorithm elsewhere and
	 *	know exactly how to set it up and use it, there's simply not enough time for it
	 *	in this project.
	 *
	 *	To make JIT at least a little bit useful, a simple 1:1 register allocator takes its place.
	 *	If the method is simple enough, used registers will fit in machine ones and translation
	 *	will be possible. It is a very limited JIT compiler, but it is at least something,
	 *	and its performance gain can be measured.
	 *
	 *
	 *	I'm purposefully leaving "TODO" comments where parts of the Linear Scan algorithm
	 *	would have been hooked if there was enough time for it.
	 *
	 *  That's also the reason for analyzing the bytecode like this instead uf just trusting
	 *	the register count filled into QuaMethod by the compiler - the analysis would have been
	 *	necessary anyway.
	 *
	 *	-- Pavel
	 */

	uint16_t maxReg = 0;
	for(list<Instruction*>::iterator it = insns.begin(); it != insns.end(); ++it) {
		switch((*it)->op) {
			// ARG0 is a reg
			case OP_LDC:
			case OP_LDCT:
			case OP_LDNULL:
			case OP_LDMYF:
			case OP_LDSTAT:
			case OP_LDS:
			case OP_CALLMY:
			case OP_NEW:
				// TODO: Liveness analysis - reg[ARG0] overwritten
			case OP_RET:
			case OP_THROW:
				updateMax(maxReg, (*it)->ARG0);
				break;

			// ARG1 is a reg
			case OP_STMYF:
				updateMax(maxReg, (*it)->ARG1);
				break;

			// ARG2 is a reg
			case OP_STS:
				updateMax(maxReg, (*it)->ARG1);
				break;

			// ARG0 and ARG1 are regs
			case OP_XCHG:
				// TODO: Liveness analysis - swap variable indices
			case OP_MOV:
			case OP_LDF:
			case OP_NEG:
			case OP_LNOT:
			case OP_IDXI:
			case OP_CALL:
			case OP_INSTOF:
			case OP_ISTYPE:
			case OP_CNVT:
				// TODO: Liveness analysis - reg[ARG0] overwritten
				updateMax(maxReg, (*it)->ARG0);
				updateMax(maxReg, (*it)->ARG1);
				break;

			// ARG0 and ARG2 are regs
			case OP_STF:
			case OP_IDXWI:
				updateMax(maxReg, (*it)->ARG0);
				updateMax(maxReg, (*it)->ARG2);
				break;

			// All 3 ARGs are regs
			case OP_A3REG:
			case OP_IDX:
				// TODO: Liveness analysis - reg[ARG0] overwritten
			case OP_IDXW:
				updateMax(maxReg, (*it)->ARG0);
				updateMax(maxReg, (*it)->ARG1);
				updateMax(maxReg, (*it)->ARG2);
				break;

			// Subop-dependent
			case OP_POP:
				// TODO: Liveness analysis - another switch, all popped is overwritten.
			case OP_PUSH:
				switch((*it)->subop) {
					case SOP_STACK_RANGE:
						updateMax(maxReg, (*it)->ARG1);
						break;
					case SOP_STACK_3:	updateMax(maxReg, (*it)->ARG2);	// intentional fall-through
					case SOP_STACK_2:	updateMax(maxReg, (*it)->ARG1);
					case SOP_STACK_1:	updateMax(maxReg, (*it)->ARG0);
				}
				break;

			case OP_JMP:
				if((*it)->subop != SOP_UNCONDITIONAL) {
					updateMax(maxReg, (*it)->ARG1);
				}
				break;

			// No register field access
			default: break; // NOP, HLT, HCF, PUSHC(T), RETT, RETNULL, TRY, CATCH, THROWT, FIN
		}

		if (maxReg > MACHINE_MAX_REG) {
			#ifdef TRACE
			cout << "too many used registers, ";
			#endif
			throw GiveUpException();
		}
	}

	// TODO: Full liveness analysis and the actual Linear Scan pass.

	map<uint16_t, MachineRegister> allocation;

	// TODO: Use Linear Scan's output to map unspilled virtual registers to those 12 machine ones

	MachineRegister regOrder[] = { REG_RCX, REG_RDX, REG_RSI, REG_RDI,
								   REG_R8, REG_R9, REG_R10, REG_R11, REG_R12, REG_R13, REG_R14, REG_R15 };
	for(int i = 0; i <= maxReg; i++) {
		allocation[i] = regOrder[i];
	}
	allocation[REG_DEV_NULL] = REG_RAX; // for service use anyway, no problem overwriting it after bogus calls

	// TODO: Also return stack locations for spilled virtual registers.

	return allocation;
}


inline void JITCompiler::append(vector<unsigned char>& buffer, const char* data, size_t count) {
	buffer.insert(buffer.end(), (const unsigned char*)data, (const unsigned char*)data + count);
}

void JITCompiler::emitOneByteInsn(MachineRegister reg, unsigned char machineOp, vector<unsigned char>& buffer) {
	if(reg >= REG_R8) {
		buffer.push_back(0x41); // REX
	}
	machineOp |= (reg % 8);
	buffer.push_back(machineOp);
}

void JITCompiler::emitTwoRegInsn(MachineRegister regRM, MachineRegister regR, // usually Dest and Src, depends on opcode
				unsigned char opcode, vector<unsigned char>& buffer, bool directAddressing, int32_t displacement) {

	unsigned char REX = 0x48;
	if(regRM >= REG_R8) {
		REX |= 0x1;
	}
	if(regR >= REG_R8) {
		REX |= 0x4;
	}

	buffer.push_back(REX);
	buffer.push_back(opcode);

	unsigned char modRM;
	bool useDisplacement = false;

	if(directAddressing) {
		modRM = 0xC0;
	} else if (displacement == 0 && regRM != REG_RSP && regRM != REG_RBP && regRM != REG_R12 && regRM != REG_R13) {
		modRM = 0x0;
	} else if (displacement >= CHAR_MIN && displacement <= CHAR_MAX) {
		modRM = 0x40;
		useDisplacement = true;
	} else {
		modRM = 0x80;
		useDisplacement = true;
	}

	modRM |= (regR % 8) << 3;
	modRM |= regRM % 8;

	buffer.push_back(modRM);

	if(regRM == REG_RSP || regRM == REG_R12) {  // SIB required
		buffer.push_back((unsigned char)regRM); // SIB: scale and index zero, base RSP or R12
	}

	if(useDisplacement) {
		append(buffer, (const char*) &displacement, (displacement >= CHAR_MIN && displacement <= CHAR_MAX) ? 1 : 4);
	}
}


void JITCompiler::emitSwitchPointersToC(vector<unsigned char>& buffer) {

	// Restore Quack and x64 stack pointers to their original state

	append(buffer, "\x48\xB8", 2);								// mov rax, qword VMBP
	append(buffer, (char*)&ptrToVMBP, sizeof(void*));
	append(buffer, "\x48\x87\x28", 3);							// xchg rbp, [rax]

	append(buffer, "\x48\xB8", 2);								// mov rax, qword VMSP
	append(buffer, (char*)&ptrToVMSP, sizeof(void*));
	append(buffer, "\x48\x87\x20", 3);							// xchg rsp, [rax]

	append(buffer, "\x48\xB8", 2);								// mov rax, qword ASP
	append(buffer, (char*)&ptrToASP, sizeof(void*));
	append(buffer, "\x48\x89\x18", 3);							// mov [rax], rbx
}


inline void JITCompiler::emitLoadLabelToRax(void* labelPtr, vector<unsigned char>& buffer) {
	append(buffer, "\x48\xB8", 2);								// mov rax, imm64
	append(buffer, (char*)&labelPtr, sizeof(void*));
}

inline void JITCompiler::emitJumpToLabel(void* labelPtr, vector<unsigned char>& buffer) {
	emitLoadLabelToRax(labelPtr, buffer);
	append(buffer, "\xFF\xE0", 2);								// jmp rax
}


void JITCompiler::translateStackOp(Instruction* insn, unsigned char opcode,
						map<uint16_t, JITCompiler::MachineRegister> allocation, vector<unsigned char> & buffer) {
	switch(insn->subop) {
		case SOP_STACK_1:
			emitOneByteInsn(allocation[insn->ARG0], opcode, buffer);
			break;

		// TODO: P2, P3, PA

		default:
			#ifdef TRACE
			cout << "can't translate PUSH/POP subop 0x" << hex << (int)insn->subop << dec << ", ";
			#endif
			throw GiveUpException();
	}
}

inline void JITCompiler::setupLeave(vector<unsigned char>& buffer) {
	emitSwitchPointersToC(buffer);
	emitLoadLabelToRax(whatLabel, buffer);
}

inline void JITCompiler::finishLeaveReg(MachineRegister reg, void* destinationLabel, vector<unsigned char>& buffer) {
	emitTwoRegInsn(REG_RAX, reg, 0x89, buffer, false);	// mov qword [rax], r??
	emitJumpToLabel(destinationLabel, buffer);	// Leave JITted code
}

inline void JITCompiler::emitPrepareContext(map<uint16_t, MachineRegister> allocation, vector<unsigned char>& buffer) {

	// make room for saving context if needed (de facto local variable space)
	append(buffer, "\x48\x83\xEC", 3); // sub rsp, imm8
	buffer.push_back((unsigned char)(allocation.size()*8));

	// set registers to _Null to avoid confusing GC if unused context is saved
	for(map<uint16_t, JITCompiler::MachineRegister>::iterator it = allocation.begin(); it != allocation.end(); ++it) {
		if(it->first == REG_DEV_NULL) {
			continue;
		}
		emitTwoRegInsn(it->second, it->second, 0x31, buffer, true); // xor reg, reg
	}
	// slightly suoptimal for the first 4 regs (redundant prefix), but works correctly
}

inline void JITCompiler::emitContextOperation(bool save,
											  map<uint16_t, MachineRegister> allocation, vector<unsigned char>& buffer){

	unsigned char opcode = save ? 0x89 : 0x8B;
	char displacement = -8;
	for(map<uint16_t, JITCompiler::MachineRegister>::iterator it = allocation.begin(); it != allocation.end(); ++it) {

		if(it->first == REG_DEV_NULL) {
			continue;
		}
		emitTwoRegInsn(REG_RBP, it->second, opcode, buffer, false, displacement);
		displacement -= 8;
	}
}


void* jitcall(void* retaddr, uint64_t that, uint64_t sigindex) {
	QuaSignature* sig = (QuaSignature*)getCurrentCPEntry((uint16_t)sigindex);
	try {
		QuaMethod* method = getClassFromValue(*(QuaValue*)&that)->lookupMethod(sig);
		interpreter->functionPrologue(*(QuaValue*)&that, method, retaddr, false, 0 /*ignored*/);
		*((QuaMethod**)JITCompiler::whatLabel) = method;
		return JITCompiler::callLabel;

	} catch(NoSuchMethodException& e) {
		*((QuaValue*)JITCompiler::whatLabel) = constructException(CLASS_NO_SUCH_METHOD_EXCEPTION, e.what());
		return JITCompiler::throwLabel;
	}
}

inline void JITCompiler::emitCall(	map<uint16_t, MachineRegister> allocation, vector<unsigned char>& buffer,
									uint16_t sigIndex, MachineRegister destReg,
									MachineRegister thatMoveRegRM, unsigned char thatMoveOpcode,
									bool thatMoveDirectAddressing, int32_t thatMoveDisplacement) {

	emitContextOperation(true, allocation, buffer);

	// For some reason unknown to me, g++ uses some kind of register-call convention on x86_64
	// without a way to turn it off
	// (extern "C", __attribute((cdecl)) and __attribute((regparm(0))) all do nothing.
	// This means my JIT will play ball and pass the 3 params to jitcall() in rdi, rsi and rdx.

	// This part sets up rdx, next is bytecode-instruction dependent rsi,

	buffer.push_back(0xBA);											// mov edx, imm32; param 3 (sigIdx)
	uint32_t imm32 = sigIndex;
	append(buffer, (const char*)&imm32, 4);

	emitTwoRegInsn(thatMoveRegRM, REG_RSI, thatMoveOpcode, buffer, thatMoveDirectAddressing, thatMoveDisplacement);
																	// mov rsi, [whatever + ?] ; param 2 (that)

	append(buffer, "\x48\x8D\x3D", 3);								// lea rdi, [rip + disp32]
	int32_t disp32 = 53; // VOLATILE! Count of bytes from the end of this instruction to landing zone.
	append(buffer, (const char*)&disp32, 4);						//		param 1 (retAddr)

	emitSwitchPointersToC(buffer);

	append(buffer, "\x48\xB8", 2);									// mov rax, imm64
	void* ptrToJitCall = __extension__ (void*)jitcall;
	append(buffer, (const char*)&ptrToJitCall, sizeof(void*));
	append(buffer, "\xFF\xD0", 2);									// call rax ( jitcall() )

	append(buffer, "\xFF\xE0", 2);									// jmp rax (label returned by jitcall)

	// Post-call RET landing zone. RAX contains return value.
	emitContextOperation(false, allocation, buffer);
	emitTwoRegInsn(destReg, REG_RAX, 0x89, buffer, true);
}


void JITCompiler::generate(list<Instruction*> insns, map<uint16_t, MachineRegister> allocation,
						   vector<unsigned char>& buffer ) {

	emitPrepareContext(allocation, buffer);

	for(list<Instruction*>::iterator it = insns.begin(); it != insns.end(); ++it) {

		switch((*it)->op) {
			case OP_NOP:
				//buffer.push_back(0x90); // only used for testing, otherwise not necessary
				break;

			case OP_MOV:
				emitTwoRegInsn(allocation[(*it)->ARG0], allocation[(*it)->ARG1], 0x89, buffer, true);
				break;

			case OP_XCHG:
				emitTwoRegInsn(allocation[(*it)->ARG1], allocation[(*it)->ARG0], 0x87, buffer, true);
				break;

			case OP_PUSH:
				translateStackOp(*it, 0x50, allocation, buffer);
				break;

			case OP_POP:
				translateStackOp(*it, 0x58, allocation, buffer);
				break;

			case OP_LDS:
				emitTwoRegInsn(REG_RBP, allocation[(*it)->ARG0], 0x8B, buffer, false, ((int16_t)(*it)->ARG1) * 8);
				break;

			case OP_CALL:
				emitCall(allocation, buffer,(*it)->ARG2,allocation[(*it)->ARG0],allocation[(*it)->ARG1],0x8B,true,0);
				break;

			case OP_CALLMY:
				emitCall(allocation, buffer, (*it)->ARG1, allocation[(*it)->ARG0], REG_RBP, 0x8B, false, 0);
				break;

			case OP_RET:
				setupLeave(buffer);
				finishLeaveReg(allocation[(*it)->ARG0], returnLabel, buffer);
				break;

			case OP_RETNULL:
				setupLeave(buffer);
				append(buffer, "\x48\xC7\x00\x00\x00\x00\x00", 7);			// mov qword [rax], 0
				emitJumpToLabel(returnLabel, buffer);						// Leave JITted code
				break;

			case OP_THROW:
				setupLeave(buffer);
				finishLeaveReg(allocation[(*it)->ARG0], throwLabel, buffer);
				break;


			default:
				#ifdef TRACE
				cout << "can't translate 0x" << hex << (int)(*it)->op << dec << ", ";
				#endif
				throw GiveUpException();
		}
	}
}

