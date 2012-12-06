#include "JITCompiler.h"
#include "bytecode.h"

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

// Constructor defined in Interpreter.cpp


#define MACHINE_MAX_REG 7
// Means allocating to 8 registers.
// Only R8-R15 for simplicity of generating code.
// Could be expanded to include RSI, RDI and maybe RCX and RDX.
// RBP ~ BP, RSP ~ SP, RBX ~ ASP, RAX reserved for service use.


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
		allocateRegisters(insns);
		generate(insns, buffer);

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

void JITCompiler::allocateRegisters(list<Instruction*> insns) {

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
				updateMax(maxReg, (*it)->ARG0);
				updateMax(maxReg, (*it)->ARG2);
				break;

			// All 3 ARGs are regs
			case OP_A3REG:
			case OP_IDX:
				// TODO: Liveness analysis - reg[ARG0] overwritten
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
}


inline void append(vector<unsigned char>& buffer, const char* data, size_t count) {
	buffer.insert(buffer.end(), (const unsigned char*)data, (const unsigned char*)data + count);
}

void JITCompiler::generate(list<Instruction*> insns, std::vector<unsigned char> & buffer ) {


	for(list<Instruction*>::iterator it = insns.begin(); it != insns.end(); ++it) {

		switch((*it)->op) {
			case OP_NOP:
				//append(buffer, "\x90", 1); // only used for testing, otherwise not necessary
				break;

			case OP_RETNULL:

				append(buffer, "\x48\xB8", 2);								// mov rax, qword whatLabel
				append(buffer, (char*)&whatLabel, sizeof(whatLabel));

				append(buffer, "\x48\xC7\x00\x00\x00\x00\x00", 7);			// mov qword [rax], 0

				append(buffer, "\x48\xB8", 2);								// mov rax, qword returnLabel
				append(buffer, (char*)&returnLabel, sizeof(returnLabel));

				append(buffer, "\xFF\xE0", 2);								// jmp rax

				break;



			default:
				#ifdef TRACE
				cout << "can't translate 0x" << hex << (*it)->op << dec << ", ";
				#endif
				throw GiveUpException();
		}
	}
}

