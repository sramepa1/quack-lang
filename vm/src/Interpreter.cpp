#include "Interpreter.h"
#include "JITCompiler.h"
#include "Exceptions.h"
#include "bytecode.h"
#include "globals.h"
#include "helpers.h"
#include "runtime.h"

#include "StringNative.h"

#include <iostream>
#include <sstream>
#include <iomanip>
#include <stdexcept>
#include <cstdlib>

using namespace std;


// Top-level functions have been moved to the end to make inlining work (g++ must know the definition to inline a call).




Interpreter::Interpreter(bool jit) : regs(vector<QuaValue>(65536)), compiler(new JITCompiler(jit))
{
	methodRegCounts.insert(0);
}


// ------------- Tracing helpers -----------------

#ifdef TRACE
#define CURRENT_METHOD_SIG getThisClass()->getName() << "::"	\
	<< ASP->currMeth->sig->name << '(' << (int)ASP->currMeth->sig->argCnt << ')'

inline char getTagMnemonic(uint8_t tag) {
	switch(tag) {
		case SOP_TAG_NONE: return 'R'; // reference
		case SOP_TAG_BOOL: return 'B';
		case SOP_TAG_INT: return 'I';
		case SOP_TAG_FLOAT: return 'F';
		default: return '?';
	}
}

inline const char* getArithmMnemonic(unsigned char subop) {
	switch(subop) {
		case SOP_ADD:	return "ADD";
		case SOP_SUB:	return "SUB";
		case SOP_MUL:	return "MUL";
		case SOP_DIV:	return "DIV";
		case SOP_MOD:	return "MOD";
		case SOP_EQ:	return "EQ";
		case SOP_NEQ:	return "NEQ";
		case SOP_GT:	return "GT";
		case SOP_GE:	return "GE";
		case SOP_LT:	return "LT";
		case SOP_LE:	return "LE";
		case SOP_LAND:	return "LAND";
		case SOP_LOR:	return "LOR";
		default:		return "???";
	}
}

inline const char* getCCMnemonic(unsigned char subop) {
	switch(subop) {
		case SOP_UNCONDITIONAL:	return "MP";
		case SOP_CC_NULL:		return "NULL";
		case SOP_CC_NNULL:		return "NNULL";
		case SOP_CC_TRUE:		return "TRUE";
		case SOP_CC_FALSE:		return "FALSE";
		default:				return "???";
	}
}
#endif


// ------------- Internal interpreter methods -----------------



void Interpreter::functionPrologue(QuaValue that, QuaMethod* method,
										  void* retAddr, bool interpretedOrigin, uint16_t destReg) {

	// push that
	*(--VMSP) = that;
	QuaValue* oldBP = VMBP;
	VMBP = VMSP;

	if(method->action <= QuaMethod::COMPILE) { // compile may fail-> save context anyway
		// note how many registers this says it may use (for GC)
		methodRegCounts.insert(method->regCount);

		// save context depending on the callee and null-out his registers
		int32_t regCnt = method->regCount;
		for(int32_t i = 0; i < regCnt; i++) {
			*(--VMSP) = regs[i];
		}
	}

	*(--ASP) = QuaFrame(retAddr, method, interpretedOrigin, destReg, (QuaValue*)valStackHigh - oldBP);
}


inline void Interpreter::functionEpilogue() {

	int32_t regCnt = ASP->currMeth->regCount;

	if(ASP->currMeth->action <= QuaMethod::COMPILE) {

		VMSP = VMBP - regCnt; // discard any pushed locals and this									// add esp, N

		// update max register count (for GC)
		methodRegCounts.erase(regCnt);

		// Restore context saved when this was called
		for(int32_t i = regCnt - 1; i >= 0; i--) {
			regs[i] = *(VMSP++);
		}
	}

	VMSP = VMBP + (ASP->ARG_COUNT + 1 /*this*/);													// mov esp, ebp
	VMBP = (QuaValue*)valStackHigh - ASP->BP_OFFSET;												// pop ebp
}

inline void Interpreter::clearContext(int32_t regCnt) {
	for(int32_t i = 0; i < regCnt; i++) {
		regs[i] = QuaValue();
	}
}

// inline wrappers for the JIT-friendly monstrosity

inline Instruction* Interpreter::performReturn(QuaValue retVal) {

	return transferControl(REASON_RETURN, *(uint64_t*)&retVal);
}


inline Instruction* Interpreter::performThrow(QuaValue& qex) {

	return transferControl(REASON_THROW, *(uint64_t*)&qex);
}


inline Instruction* Interpreter::performCall(QuaMethod* method) {

	return transferControl(REASON_CALL, (uint64_t)method);
}


// ------------------------- JIT interface -------------------------- //
// This will be the stack frame in which jitted code is executing.
// That means it can't be refactored into multiple functions, as code flow leaves to jitted code in one switch branch,
//	but comes back through another one via asm goto

// The switch branches do not share any variables except a single volatile static one, which is only ever used
// at the beginning of a branch, essentially forming a simple calling convention within one C stack frame.

// The 'asm goto' construction available since g++ 4.5 is what makes this possible to be done safely.
// The compiler knows that the MACHINE_JUMP asm blocks will always jump to one of the three labels
// and may clobber any register, and the compiler can arrange things around this.

// local macro only, #undef'd at the end of the method
#define MACHINE_JUMP(value, code) {												\
						volatile void* destination = (volatile void*)(code);	\
						asm goto (	"leaq VMSP, %%rbx\n\t"						\
									"xchgq %%rsp, (%%rbx)\n\t"					\
									"leaq VMBP, %%rbx\n\t"						\
									"xchgq %%rbp, (%%rbx)\n\t"					\
									"jmp *%%rdx"								\
						:	/* no output */										\
						:	"a" (value), "d" (destination)						\
						:	"memory", "cc" , "rbx", "rcx", "rsi", "rdi",		\
							"r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15"\
						:	landing_call, landing_return , landing_throw);		\
					}

// JIT compiler needs to know the address of 3 labels and one variable.
// I'd rather not make them global, so I define the constructor here to make them share a translation unit.
JITCompiler::JITCompiler(bool enabled) : enabled(enabled)
{
	// get adresses of asm-jump labels and "what" in Interpreter::transferControl(reason, what)
	asm("mov $transfer_call, %%rax" : "=a" (callLabel) : : );
	asm("mov $transfer_return, %%rax" : "=a" (returnLabel) : : );
	asm("mov $transfer_throw, %%rax" : "=a" (throwLabel) : : );
	asm("mov $transfer_what, %%rax" : "=a" (whatLabel) : : );

	ptrToVMBP = &VMBP;
	ptrToVMSP = &VMSP;
}

__attribute((noinline)) Instruction* Interpreter::transferControl(TransferReason reason, uint64_t param) {

	//
	static volatile uint64_t volatile_what asm("transfer_what");

	volatile_what = param;

	switch(reason) {

		// -------------- perform a call -------------- //

		case REASON_CALL:
		{

			// C label for "asm goto" to let g++ know jitted code might perform a jump here
			landing_call:

			// define an asm label for the actual landing (should form 2 labels pointing to the same address)
			asm volatile (".local transfer_call\n\t"\
						  "transfer_call: nop" ::: "memory");
						// (fake memory clobber so that this isn't moved - volatile only guarantees no deletion)

			QuaMethod* method = (QuaMethod*)volatile_what;

			#ifdef TRACE
				cout << "# Calling " << CURRENT_METHOD_SIG << " [";
			#endif

			switch(method->action) {
				case QuaMethod::INTERPRET:
					#ifdef TRACE
						cout << "interpreting]" << endl;
					#endif
					method->action = QuaMethod::COMPILE; // next time
					clearContext(method->regCount);
					return (Instruction*) method->code;

				case QuaMethod::COMPILE:
					#ifdef TRACE
						cout << "compiling... ";
					#endif

					if(compiler->compile(method)) {

						// discard saved context and GC info
						// - not needed when compilation was successful and the method will not be interpreted
						methodRegCounts.erase(method->regCount);
						VMSP += method->regCount;


						method->action = QuaMethod::JUMPTO;

					} else {
					#ifdef TRACE
						cout << "giving up]" << endl;
					#endif
						method->action = QuaMethod::ALWAYS_INTERPRET;
						clearContext(method->regCount);
						return (Instruction*) method->code; // No further setting of compile flag
					}

					// success = fall-through to execution

				case QuaMethod::JUMPTO:
					#ifdef TRACE
						cout << "jumping to start]" << endl;
					#endif

					// the 0 will be ignored
					MACHINE_JUMP(0, method->code)
					// no return

				case QuaMethod::C_CALL:
					#ifdef TRACE
						cout << "native call]" << endl;
					#endif
					try {

						// perform a return with what the native method returns
						QuaValue retVal = ( __extension__ (QuaValue (*)())method->code )();

						// avoid C stack creep by jumping to the return part of this function immediately
						volatile_what = *(uint64_t*)&retVal;
						goto landing_return;

						// Explanation of the goto: There used to be a "performReturn" call here,
						// but if that returns to jitted code, it is now one C stack frame above what it used to be.
						// The only way of avoiding this is behaving like JIT itself and jumping to labels
						// while passing the parameter through the volatile variable.


					} catch (QuaValue qex) {

						// same as above, different mechanism

						volatile_what = *(uint64_t*)&qex;
						goto landing_throw;
					}

				case QuaMethod::ALWAYS_INTERPRET:
					#ifdef TRACE
						cout << "interpreting - forced]" << endl;
					#endif
					clearContext(method->regCount);
					return (Instruction*) method->code;

				default:
					ostringstream os;
					os << "Corrupted method action detected: " << method->action;
					throw runtime_error(os.str());
			}
		}


		// -------------- perform a return -------------- //

		case REASON_RETURN:
		{
			landing_return:
			asm volatile (".local transfer_return\n\t"\
						  "transfer_return: nop" ::: "memory");

			QuaValue retVal = *(QuaValue*)&volatile_what;

			#ifdef TRACE
				cout << "# Returning from " << CURRENT_METHOD_SIG;
			#endif

			// discard adress stack junk
			while(ASP->FRAME_TYPE == EXCEPTION || ASP->FRAME_TYPE == FINALLY) {
				++ASP;
			}
			if(ASP->FRAME_TYPE == EXIT) {
				#ifdef TRACE
					cout << " ...and halting the VM." << endl;
				#endif
				throw ExitException(); // TODO Is this safe if code had passed through ASM?
			}

			// The frame is a correct return address (frameType == METHOD)
			functionEpilogue();

			if(ASP->INTERPRETED) {
				regs[ASP->DEST_REG] = retVal;
				Instruction* retAddr = (Instruction*) (ASP++)->retAddr;

				#ifdef TRACE
						cout << " to " << CURRENT_METHOD_SIG << " [interpreting]" << endl;
				#endif
				return retAddr;

			} else {

				void* code = (ASP++)->retAddr;

				#ifdef TRACE
						cout << " to " << CURRENT_METHOD_SIG << " [jumping]" << endl;
				#endif

				// return value passed through rax, JIT-generated stub will move it to destination register.
				MACHINE_JUMP(retVal, code)
				// no return
			}
		}


		// -------------- perform a throw -------------- //

		case REASON_THROW:
		{
			landing_throw:
			asm volatile (".local transfer_throw\n\t"\
						  "transfer_throw: nop" ::: "memory");

			QuaValue qex = *(QuaValue*)&volatile_what;

			#ifdef TRACE
				cout << "# Throwing an exception of class " << getClassFromValue(qex)->getName()
				   << " from " << CURRENT_METHOD_SIG;
			#endif

			while(1) {
				if(ASP->FRAME_TYPE == EXIT) {
					return unhandledException(qex); // throws a runtime_error

				} else if (ASP->FRAME_TYPE == EXCEPTION && instanceOf(qex, ASP->EXCEPTION_TYPE)) {
					// correct handler
					break;
				}

				// ASP points to junk -> discard it
				if(ASP->FRAME_TYPE == METHOD) {
					// unwind value stack and restore saved context:
					functionEpilogue();
				}
				++ASP;
			}

			if(ASP->INTERPRETED) {

				// push exception
				*(--VMSP) = qex;

				#ifdef TRACE
					cout << " to a handler in " << CURRENT_METHOD_SIG << "[interpreting]" << endl;
				#endif
				return (Instruction*)(ASP++)->retAddr;

			} else {

				#ifdef TRACE
					cout << " to a handler in " << CURRENT_METHOD_SIG << "[jumping]" << endl;
				#endif

				// exception value passed through rax, JIT-generated stub will push it after restoring context.
				void* code = (ASP++)->retAddr;
				MACHINE_JUMP(qex, code)
				// no return
			}
		}


		// -- invalid reason -- //

		default:
			ostringstream os;
			os << "Corrupted control transfer reason: " << reason << '.' << endl
			   << "This is most likely a JIT bug, try running the VM with -nojit.";
			throw runtime_error(os.str());
	}
}

#undef MACHINE_JUMP

// --------------------- End of JIT interface -------------------------- //





inline Instruction* Interpreter::commonException(const char* className, const char* what) {
	QuaValue qex = constructException(className, what);
	return performThrow(qex);
}

inline Instruction* Interpreter::directCall(uint16_t destReg, QuaValue& that, QuaSignature* sig, Instruction* retAddr) {
	QuaMethod* method;
	try {
		method = getClassFromValue(that)->lookupMethod(sig);

	} catch(NoSuchMethodException& e) {
		return commonException(CLASS_NO_SUCH_METHOD_EXCEPTION, e.what());
	} catch(NoSuchClassException& e) {
		return commonException(CLASS_NO_SUCH_CLASS_EXCEPTION, e.what());
	}
	functionPrologue(that, method, retAddr, true, destReg);
	return performCall(method);
}


inline Instruction* Interpreter::commonCall(uint16_t destReg, QuaValue& that, uint16_t sigIndex, Instruction* retAddr) {
	return directCall(destReg, that, (QuaSignature*)getCurrentCPEntry(sigIndex), retAddr);
}


inline Instruction* Interpreter::handleIllegalInstruction(Instruction* insn) {
#ifdef TRACE
	cout << "Illegal instruction!" << endl; // terminate the open line from main loop
#endif
	ostringstream os;
	os  << "Illegal instruction opcode 0x" << setw(2)<<setfill('0')<<uppercase<<hex << (int)insn->op
		<< " encountered at " << insn << " in class " << getThisClass()->getName() << "." << endl;
	throw runtime_error(os.str());
}

inline Instruction* Interpreter::handleIllegalSubOp(Instruction* insn) {
#ifdef TRACE
	cout << "Illegal sub-instruction!" << endl;
#endif
	ostringstream os;
	os  << "Illegal sub-opcode 0x" << setw(2)<<setfill('0')<<uppercase<<hex << (int)insn->subop << endl
		<< "encountered in an instruction with opcode 0x " << setw(2)<<setfill('0')<<uppercase<<hex << (int)insn->op
		<< endl << "at address " << insn << " in class " << getThisClass()->getName() << "." << endl;
	throw runtime_error(os.str());
}


Instruction* Interpreter::unhandledException(QuaValue qex) {
	#ifdef TRACE
		cout << " ...no suitable handler found!" << endl;
	#endif

	ostringstream os;
	os << "Unhandled exception of class " << getClassFromValue(qex)->getName() << endl;

	// Attempt to extract qex.what
	try {
		try { // it is most likely an Exception and has a 'what' field - duck-typed extraction approach
			QuaValue qwhat = getFieldByName(qex, "what");
			if(qwhat.type != typeCache.typeString) {
				throw runtime_error("'what' field is not a string.");
			}
			os << stringSerializer(qwhat);

		} catch(NoSuchFieldException& e) { // not an Exception - at least try stringValue...

			try {
				os << stringSerializer(nativeCall(qex, (QuaSignature*)"\0_stringValueN"));
				// TODO: this will fail if it is not a runtime native type:
				// Solve how to restart interpreter to evaluate a _stringValue loop
				// And what to do with unhandled exceptions possibly produced in that loop...

			} catch(NoSuchMethodException& e2) {
				os << "Neither 'what' nor '_stringValue' could be extracted from the thrown exception." << endl
				   << "No message available.";
			}
		}

	} catch(runtime_error& e) {
		os << "Error when extracting exception message:" << endl << e.what();
	}

	throw runtime_error(os.str());
}


inline QuaValue Interpreter::extractTaggedValue(Instruction* insn) {
	return QuaValue(insn->IMM, getTaggedType(insn->subop), insn->subop);
}



// ------------- Instruction handlers -----------------





inline Instruction* Interpreter::handleNOP(Instruction* insn) {

#ifdef TRACE
	cout << "NOP" << endl;
#endif

	return ++insn;
}


inline Instruction* Interpreter::handleLDC(Instruction* insn) {

#ifdef TRACE
	cout << "LDC r" << insn->ARG0 << ", " << insn->ARG1 << ", " << insn->ARG2 << endl;
#endif

	try {
		regs[insn->ARG0] = loadConstant(insn->ARG1, insn->ARG2);
	} catch(NoSuchClassException& e) {
		return commonException(CLASS_NO_SUCH_CLASS_EXCEPTION, e.what());
	}
	return ++insn;
}


inline Instruction* Interpreter::handleLDCT(Instruction* insn) {

#ifdef TRACE
	cout << "LDC" << getTagMnemonic(insn->subop) << " r" << insn->REG << ", 0x" << hex << insn->IMM << dec << endl;
#endif

	regs[insn->REG] = extractTaggedValue(insn);
	return ++insn;
}


inline Instruction* Interpreter::handleLDSTAT(Instruction* insn) {

#ifdef TRACE
	cout << "LDSTAT r" << insn->ARG0 << ", " << insn->ARG1 << endl;
#endif

	QuaClass* statclass;
	try {
		statclass = getClassFromType(insn->ARG1); // also resolves type
	} catch(NoSuchClassException& e) {
		return commonException(CLASS_NO_SUCH_CLASS_EXCEPTION, e.what());
	}
	QuaValue instance = statclass->getInstance();
	if(instance.value == 0) {
		instance = newRawInstance(insn->ARG1);
		statclass->setInstance(instance);
		return directCall(insn->ARG0, instance, (QuaSignature*)"\0init", insn);
	}
	regs[insn->ARG0] = instance;
	return ++insn;
}


inline Instruction* Interpreter::handleLDNULL(Instruction* insn) {

#ifdef TRACE
	cout << "LDNULL r" << insn->ARG0 << endl;
#endif

	regs[insn->ARG0] = QuaValue();
	return ++insn;
}


inline Instruction* Interpreter::handleLDF(Instruction* insn) {

#ifdef TRACE
	cout << "LDF r" << insn->ARG0 << ", r" << insn->ARG1 << ", " << insn->ARG2 << endl;
#endif

	try {
		regs[insn->ARG0] = getFieldByName(regs[insn->ARG1], getCurrentCPEntry(insn->ARG2));

	} catch (NoSuchFieldException& e) {
		return commonException(CLASS_NO_SUCH_FIELD_EXCEPTION, e.what());
	} catch (NullReferenceException& e) {
		return commonException(CLASS_NULL_REFERENCE_EXCEPTION, e.what());
	}

	return ++insn;
}


inline Instruction* Interpreter::handleSTF(Instruction* insn) {

#ifdef TRACE
	cout << "STF r" << insn->ARG0 << ", " << insn->ARG1 << ", r" << insn->ARG2 << endl;
#endif

	try {
		getFieldByName(regs[insn->ARG0], getCurrentCPEntry(insn->ARG1)) = regs[insn->ARG2];

	} catch (NoSuchFieldException& e) {
		return commonException(CLASS_NO_SUCH_FIELD_EXCEPTION, e.what());
	} catch (NullReferenceException& e) {
		return commonException(CLASS_NULL_REFERENCE_EXCEPTION, e.what());
	}

	return ++insn;
}


inline Instruction* Interpreter::handleLDMYF(Instruction* insn) {

#ifdef TRACE
	cout << "LDMYF r" << insn->ARG0 << ", " << insn->ARG1 << endl;
#endif

	try {
		regs[insn->ARG0] = getFieldByIndex(*VMBP, insn->ARG1);
	} catch (NullReferenceException& e) {
		return commonException(CLASS_NULL_REFERENCE_EXCEPTION, e.what());
	}
	return ++insn;
}


inline Instruction* Interpreter::handleSTMYF(Instruction* insn) {

#ifdef TRACE
	cout << "STMYF " << insn->ARG0 << ", r" << insn->ARG1 << endl;
#endif

	try {
		getFieldByIndex(*VMBP, insn->ARG0) = regs[insn->ARG1];
	} catch (NullReferenceException& e) {
		return commonException(CLASS_NULL_REFERENCE_EXCEPTION, e.what());
	}
	return ++insn;
}


inline Instruction* Interpreter::handleMOV(Instruction* insn) {
#ifdef TRACE
	cout << "MOV r" << insn->ARG0 << ", r" << insn->ARG1 << endl;
#endif

	regs[insn->ARG0] = regs[insn->ARG1];
	return ++insn;
}


inline Instruction* Interpreter::handleXCHG(Instruction* insn) {

#ifdef TRACE
	cout << "XCHG r" << insn->ARG0 << ", r" << insn->ARG1 << endl;
#endif

	QuaValue tmp = regs[insn->ARG0];
	regs[insn->ARG0] = regs[insn->ARG1];
	regs[insn->ARG1] = tmp;
	return ++insn;
}


inline Instruction* Interpreter::handlePUSH(Instruction* insn) {

	switch(insn->subop) {
		case SOP_STACK_1:
#ifdef TRACE
			cout << "PUSH r" << insn->ARG0 << endl;
#endif
			*(--VMSP) = regs[insn->ARG0];
			return ++insn;

		case SOP_STACK_2:
#ifdef TRACE
			cout << "PUSH2 r" << insn->ARG0 << ", r" << insn->ARG1 << endl;
#endif
			*(--VMSP) = regs[insn->ARG0];
			*(--VMSP) = regs[insn->ARG1];
			return ++insn;

		case SOP_STACK_3:
#ifdef TRACE
			cout << "PUSH3 r" << insn->ARG0 << ", r" << insn->ARG1 << ", r" << insn->ARG2 << endl;
#endif
			*(--VMSP) = regs[insn->ARG0];
			*(--VMSP) = regs[insn->ARG1];
			*(--VMSP) = regs[insn->ARG2];
			return ++insn;

		case SOP_STACK_RANGE:
#ifdef TRACE
			cout << "PUSHA r" << insn->ARG0 << ", r" << insn->ARG1 << endl;
#endif
			for(unsigned int i = insn->ARG0; i <= insn->ARG1; i++) {
				*(--VMSP) = regs[i];
			}
			return ++insn;

		default:
			return handleIllegalSubOp(insn);
	}
}


inline Instruction* Interpreter::handlePOP(Instruction* insn) {

	switch(insn->subop) {
		case SOP_STACK_1:
#ifdef TRACE
			cout << "POP r" << insn->ARG0 << endl;
#endif
			regs[insn->ARG0] = *(VMSP++);
			return ++insn;

		case SOP_STACK_2:
#ifdef TRACE
			cout << "POP2 r" << insn->ARG0 << ", r" << insn->ARG1 << endl;
#endif
			regs[insn->ARG1] = *(VMSP++);
			regs[insn->ARG0] = *(VMSP++);
			return ++insn;

		case SOP_STACK_3:
#ifdef TRACE
			cout << "POP3 r" << insn->ARG0 << ", r" << insn->ARG1 << ", r" << insn->ARG2 << endl;
#endif
			regs[insn->ARG2] = *(VMSP++);
			regs[insn->ARG1] = *(VMSP++);
			regs[insn->ARG0] = *(VMSP++);
			return ++insn;

		case SOP_STACK_RANGE:
#ifdef TRACE
			cout << "POPA r" << insn->ARG0 << ", r" << insn->ARG1 << endl;
#endif
			for(unsigned int i = insn->ARG1; i >= insn->ARG0; i--) {
				regs[i] = *(VMSP++);
			}
			return ++insn;

		default:
			return handleIllegalSubOp(insn);
	}
}


inline Instruction* Interpreter::handlePUSHC(Instruction* insn) {

#ifdef TRACE
	cout << "PUSHC " << insn->ARG0 << ", " << insn->ARG1 << endl;
#endif

	try {
		QuaValue constant = loadConstant(insn->ARG0, insn->ARG1);
		*(--VMSP) = constant;
	} catch(NoSuchClassException& e) {
		return commonException(CLASS_NO_SUCH_CLASS_EXCEPTION, e.what());
	}
	return ++insn;
}


inline Instruction* Interpreter::handlePUSHCT(Instruction* insn) {

#ifdef TRACE
	cout << "PUSHC" << getTagMnemonic(insn->subop) << ", 0x" << hex << insn->IMM << dec << endl;
#endif

	*(--VMSP) = extractTaggedValue(insn);
	return ++insn;
}


inline Instruction* Interpreter::handleLDS(Instruction* insn) {

#ifdef TRACE
	cout << "LDS r" << insn->ARG0 << ", " << insn->ARG1 << endl;
#endif

	regs[insn->ARG0] = *(VMBP + insn->ARG1);
	return ++insn;
}


inline Instruction* Interpreter::handleSTS(Instruction* insn) {

#ifdef TRACE
	cout << "STS " << insn->ARG0 << ", r" << insn->ARG1 << endl;
#endif

	*(VMBP + insn->ARG0) = regs[insn->ARG1];
	return ++insn;
}


// helper fnuction
inline bool Interpreter::taggedA3Possible(uint8_t leftTag, uint16_t rightReg) {
	return leftTag != TAG_REF && leftTag == regs[rightReg].tag;
}


// These macros are meant for handleA3REG(insn) only. They are #undef'd right after the function that uses them.
// If this were C++11, this could have been made less ugly by using lambdas instead of macros.

#define TAGGED_A3REG(QuaType, ctype, op) regs[insn->ARG0] =	\
	create##QuaType ((ctype)regs[insn->ARG1].value op (ctype)regs[insn->ARG2].value)


#define TAGSWITCH_START(tag)	if(taggedA3Possible(tag, insn->ARG2)) {			\
									switch(tag) {
#define TAGSWITCH_CASE(tag, QuaType, ctype, op) case tag:	TAGGED_A3REG(QuaType, ctype, op);	return ++insn;
#define TAGSWITCH_END(sig)						default: break;					\
									}											\
								}												\
								opSig = (QuaSignature*)sig

#define A3REG_MATH(tag, op, sig)		TAGSWITCH_START(tag)								\
											TAGSWITCH_CASE(TAG_INT, Integer, int32_t, op)	\
											TAGSWITCH_CASE(TAG_FLOAT, Float, float, op)		\
										TAGSWITCH_END(sig)

#define A3REG_EQUALITY(tag, op, sig)	TAGSWITCH_START(tag)								\
											TAGSWITCH_CASE(TAG_INT, Bool, int32_t, op)		\
											TAGSWITCH_CASE(TAG_BOOL, Bool, bool, op)		\
											TAGSWITCH_CASE(TAG_FLOAT, Bool, float, op)		\
										TAGSWITCH_END(sig)

#define A3REG_RELATIONAL(tag, op, sig)	TAGSWITCH_START(tag)								\
											TAGSWITCH_CASE(TAG_INT, Bool, int32_t, op)		\
											TAGSWITCH_CASE(TAG_FLOAT, Bool, float, op)		\
										TAGSWITCH_END(sig)


#define A3REG_ONETYPE(tag, mustbe, QuaType, ctype, op, sig) if(taggedA3Possible(tag, insn->ARG2) && tag == mustbe) {\
																TAGGED_A3REG(QuaType, ctype, op);					\
																return ++insn;										\
															}														\
															opSig = (QuaSignature*)sig

#define A3REG_INT(tag, op, sig) A3REG_ONETYPE(tag, TAG_INT, Integer, int32_t, op, sig)
#define A3REG_BOOL(tag, op, sig) A3REG_ONETYPE(tag, TAG_BOOL, Bool, bool, op, sig)

// Now, isn't this neat switch worth the macro ballast?
// The thing is, macros are the only way to pass C++ operators as parameters while still keeping code inlined.

inline Instruction* Interpreter::handleA3REG(Instruction* insn) {

#ifdef TRACE
	cout << getArithmMnemonic(insn->subop) << " r" << insn->ARG0 << ", r" << insn->ARG1 << ", r" << insn->ARG2 << endl;
#endif

	uint8_t tag = regs[insn->ARG1].tag;
	QuaSignature* opSig = NULL;

	switch(insn->subop) {
		case SOP_ADD:
			A3REG_MATH(tag, +, "\1_opPlus");
			break;
		case SOP_SUB:
			A3REG_MATH(tag, -, "\1_opMinus");
			break;
		case SOP_MUL:
			A3REG_MATH(tag, *, "\1_opMul");
			break;
		case SOP_DIV:
			A3REG_MATH(tag, /, "\1_opDiv");
			break;
		case SOP_MOD:
			A3REG_INT(tag, %, "\1_opMod");
			break;
		case SOP_EQ:
			A3REG_EQUALITY(tag, ==, "\1_opEq");
			break;
		case SOP_NEQ:
			A3REG_EQUALITY(tag, !=, "\1_opNeq");
			break;
		case SOP_GT:
			A3REG_RELATIONAL(tag, >, "\1_opGt");
			break;
		case SOP_GE:
			A3REG_RELATIONAL(tag, >=, "\1_opGe");
			break;
		case SOP_LT:
			A3REG_RELATIONAL(tag, <, "\1_opLt");
			break;
		case SOP_LE:
			A3REG_RELATIONAL(tag, <=, "\1_opLe");
			break;
		case SOP_LAND:
			A3REG_BOOL(tag, &&, "\1_opLAnd");
			break;
		case SOP_LOR:
			A3REG_BOOL(tag, ||, "\1_opLOr");
			break;
		default:
			return handleIllegalSubOp(insn);
	}

	// in case tagged operation was not possible
	*(--VMSP) = regs[insn->ARG2];
	return directCall(insn->ARG0, regs[insn->ARG1], opSig, insn + 1);
}

#undef TAGGED_A3REG
#undef TAGSWITCH_START
#undef TAGSWITCH_CASE
#undef TAGSWITCH_END
#undef A3REG_BOOL
#undef A3REG_EQUALITY
#undef A3REG_INT
#undef A3REG_MATH
#undef A3REG_ONETYPE
#undef A3REG_RELATIONAL

// macro cleanup done.


inline Instruction* Interpreter::handleNEG(Instruction* insn) {

#ifdef TRACE
	cout << "NEG r" << insn->ARG0 << ", r" << insn->ARG1 << endl;
#endif

	switch(regs[insn->ARG1].tag) {
		case TAG_BOOL:  break;
			// pretty much a guaranteed NoSuchMethodException, but included for consistency and future additions.
		case TAG_INT:   regs[insn->ARG0] = createInteger( - *((int32_t*) &(regs[insn->ARG1].value)) );
						return ++insn;
		case TAG_FLOAT: regs[insn->ARG0] = createFloat( - *((float*) &(regs[insn->ARG1].value)) );
						return ++insn;
		case TAG_REF:   break;
		default: errorUnknownTag(regs[insn->ARG1].tag);
	}

	return directCall(insn->ARG0, regs[insn->ARG1], (QuaSignature*)"\0_opUnMinus", insn + 1);;
}


inline Instruction* Interpreter::handleLNOT(Instruction* insn) {

#ifdef TRACE
	cout << "LNOT r" << insn->ARG0 << ", r" << insn->ARG1 << endl;
#endif

	switch(regs[insn->ARG1].tag) {
		case TAG_BOOL:  regs[insn->ARG0] = createBool( ! (bool) regs[insn->ARG1].value );
						return ++insn;

		case TAG_INT:   break;
			// This one may actually work after autoboxing, if someone implements the ! operator for it, like C does
			// if that ever happens, that implementation should probably be called from here for speedup.

		case TAG_FLOAT: break;
			// NoSuchMethodException for consistency

		case TAG_REF:   break;
		default: errorUnknownTag(regs[insn->ARG1].tag);
	}

	return directCall(insn->ARG0, regs[insn->ARG1], (QuaSignature*)"\0_opLNot", insn + 1);
}



inline Instruction* Interpreter::commonIDX(Instruction* insn) {
	return directCall(insn->ARG0, regs[insn->ARG1], (QuaSignature*)"\1_opIndex", insn + 1);
}


inline Instruction* Interpreter::commonIDXW(Instruction* insn) {
	return directCall(REG_DEV_NULL, regs[insn->ARG0], (QuaSignature*)"\2_opIndexW", insn + 1);
}


inline Instruction* Interpreter::handleIDX(Instruction* insn) {

#ifdef TRACE
	cout << "IDX r" << insn->ARG0 << ", r" << insn->ARG1 << ", r" << insn->ARG2 << endl;
#endif

	*(--VMSP) = regs[insn->ARG2];
	return commonIDX(insn);
}


inline Instruction* Interpreter::handleIDXI(Instruction* insn) {

#ifdef TRACE
	cout << "IDXI r" << insn->ARG0 << ", r" << insn->ARG1 << ", " << insn->ARG2 << endl;
#endif

	*(--VMSP) = QuaValue(insn->ARG2, typeCache.typeInteger, TAG_INT);
	return commonIDX(insn);
}


inline Instruction* Interpreter::handleIDXW(Instruction* insn) {

#ifdef TRACE
	cout << "IDXW r" << insn->ARG0 << ", r" << insn->ARG1 << ", r" << insn->ARG2 << endl;
#endif

	*(--VMSP) = regs[insn->ARG2]; // push written value
	*(--VMSP) = regs[insn->ARG1]; // push index
	return commonIDXW(insn);
}


inline Instruction* Interpreter::handleIDXWI(Instruction* insn) {

#ifdef TRACE
	cout << "IDXWI r" << insn->ARG0 << ", " << insn->ARG1 << ", r" << insn->ARG2 << endl;
#endif

	*(--VMSP) = regs[insn->ARG2];
	*(--VMSP) = QuaValue(insn->ARG1, typeCache.typeInteger, TAG_INT);
	return commonIDXW(insn);
}


inline bool Interpreter::isNull(QuaValue v) {
	return (v.tag == TAG_REF) && ( v.value == 0 || v.type == 0 );
}

inline bool Interpreter::parseTaggedBool(QuaValue v) {
	if(v.tag != TAG_BOOL) {
		ostringstream os;
		os << "Expected a tagged boolean, but the tag was 0x" << hex << setw(2) << setfill('0')
		   << v.tag << '.' << endl << "This is most likely a compiler bug." << endl;
		throw runtime_error(os.str());
	}
	return (bool)v.value;
}


inline Instruction* Interpreter::handleJMP(Instruction* insn) {

#ifdef TRACE
	cout << 'J' << getCCMnemonic(insn->subop) << ' ' << (int16_t)insn->ARG0;
	if(insn->subop != SOP_UNCONDITIONAL) cout << ", r" << insn->ARG1;
	cout << endl;
#endif

	bool condition;
	switch(insn->subop) {
		case SOP_UNCONDITIONAL:	condition = true; break;
		case SOP_CC_NULL:		condition = isNull(regs[insn->ARG1]); break;
		case SOP_CC_NNULL:		condition = !isNull(regs[insn->ARG1]); break;
		case SOP_CC_TRUE:		condition = parseTaggedBool(regs[insn->ARG1]); break;
		case SOP_CC_FALSE:		condition = !parseTaggedBool(regs[insn->ARG1]); break;
		default: return handleIllegalSubOp(insn);
	}
	return condition ? insn + 1 + (int16_t)insn->ARG0 : ++insn;
}


inline Instruction* Interpreter::handleCALL(Instruction* insn) {

#ifdef TRACE
	cout << "CALL r" << insn->ARG0 << ", r" << insn->ARG1 << ", " << insn->ARG2 << endl;
#endif

	return commonCall(insn->ARG0, regs[insn->ARG1], insn->ARG2, insn + 1);
}


inline Instruction* Interpreter::handleCALLMY(Instruction* insn) {

#ifdef TRACE
	cout << "CALLMY r" << insn->ARG0 << ", " << insn->ARG1 << endl;
#endif

	return commonCall(insn->ARG0, *VMBP, insn->ARG1, insn + 1);
}


// TODO: make ARG2 param count to hard-coded "init" or use it as constructor signature CP index?
inline Instruction* Interpreter::handleNEW(Instruction* insn) {

#ifdef TRACE
	cout << "NEW r" << insn->ARG0 << ", " << insn->ARG1 << ", " << insn->ARG2 << endl;
#endif

	try {
		regs[insn->ARG0] = newRawInstance(insn->ARG1);
	} catch(NoSuchClassException& e) {
		return commonException(CLASS_NO_SUCH_CLASS_EXCEPTION, e.what());
	}
	return commonCall(REG_DEV_NULL, regs[insn->ARG0], insn->ARG2, insn + 1); // temporarily use CP index
}


// Compiler writers beware: RET does not restore any saved context!
inline Instruction* Interpreter::handleRET(Instruction* insn) {

#ifdef TRACE
	cout << "RET r" << insn->ARG0 << endl;
#endif

	return performReturn(regs[insn->ARG0]);
}


inline Instruction* Interpreter::handleRETT(Instruction* insn) {

#ifdef TRACE
	cout << "RET" << getTagMnemonic(insn->subop) << ", 0x" << hex << insn->IMM << dec << endl;
#endif

	return performReturn(extractTaggedValue(insn));
}

inline Instruction* Interpreter::handleRETNULL() {

#ifdef TRACE
	cout << "RETNULL" << endl;
#endif

	return performReturn(QuaValue());
}


inline Instruction* Interpreter::handleTRY(Instruction* insn) {

#ifdef TRACE
	cout << "TRY " << (int16_t)insn->ARG0 << endl;
#endif

	QuaMethod* currMeth = ASP->currMeth;
	*(--ASP) = QuaFrame(insn + 1 + (int16_t)insn->ARG0, currMeth, true, true);
	return ++insn;
}


inline Instruction* Interpreter::handleCATCH(Instruction* insn) {

#ifdef TRACE
	cout << "CATCH " << insn->ARG0 << ", " << (int16_t)insn->ARG1 << endl;
#endif

	QuaMethod* currMeth = ASP->currMeth;
	try {
		QuaFrame newFrame = QuaFrame(insn + 1 + (int16_t)insn->ARG1, currMeth, true, resolveType(insn->ARG0));
		*(--ASP) = newFrame; // only push when constructed correctly to avoid corrupting the A-stack
	} catch(NoSuchClassException& e) {
		return commonException(CLASS_NO_SUCH_CLASS_EXCEPTION, e.what());
	}

	return ++insn;
}


inline Instruction* Interpreter::handleTHROW(Instruction* insn) {

#ifdef TRACE
	cout << "THROW r" << insn->ARG0 << endl;
#endif

	return performThrow(regs[insn->ARG0]);
}


inline Instruction* Interpreter::handleTHROWT(Instruction* insn) {

#ifdef TRACE
	cout << "THROW" << getTagMnemonic(insn->subop) << ", 0x" << hex << insn->IMM << dec << endl;
#endif

	QuaValue qex = extractTaggedValue(insn);
	return performThrow(qex);
}


inline Instruction* Interpreter::handleFIN(Instruction* insn) {

#ifdef TRACE
	cout << "FIN" << endl;
#endif

	// discard unused handlers
	while(ASP->FRAME_TYPE == EXCEPTION) {
		++ASP;
	}
	if(ASP->FRAME_TYPE != FINALLY) {
		throw runtime_error("FINALLY instruction used without a preceding TRY.");
	}

	return (Instruction*)((ASP++)->retAddr);
}


inline Instruction* Interpreter::handleINSTOF(Instruction* insn) {

#ifdef TRACE
	cout << "INSTOF r" << insn->ARG0 << ", r" << insn->ARG1 << ", "  << insn->ARG2 << endl;
#endif

	try {
		regs[insn->ARG0] = createBool(instanceOf(regs[insn->ARG1], insn->ARG2));
	} catch(NoSuchClassException& e) {
		return commonException(CLASS_NO_SUCH_CLASS_EXCEPTION, e.what());
	}
	return ++insn;
}


inline Instruction* Interpreter::handleISTYPE(Instruction* insn) {

#ifdef TRACE
	cout << "ISTYPE r" << insn->ARG0 << ", r" << insn->ARG1 << ", "  << insn->ARG2 << endl;
#endif

	try {
		regs[insn->ARG0] = createBool(resolveType(regs[insn->ARG1].type) == resolveType(insn->ARG2));
	} catch(NoSuchClassException& e) {
		return commonException(CLASS_NO_SUCH_CLASS_EXCEPTION, e.what());
	}
	return ++insn;
}


inline Instruction* Interpreter::handleCNVT(Instruction* insn) {

#ifdef TRACE
	cout << "CNVT" << getTagMnemonic(insn->subop) << " r" << insn->ARG0 << ", r" << insn->ARG1 << endl;
#endif

	if(regs[insn->ARG1].tag == insn->subop) {
		regs[insn->ARG0] = regs[insn->ARG1];
		return ++insn;
	} else {
		QuaSignature* sig = NULL;
		switch(insn->subop) {
			case SOP_TAG_BOOL:  sig = (QuaSignature*)"\0_boolValue"; break;
			case SOP_TAG_FLOAT:  sig = (QuaSignature*)"\0_floatValue"; break;
			case SOP_TAG_INT:  sig = (QuaSignature*)"\0_intValue"; break;
			case SOP_TAG_NONE: return ++insn;
				// Nothing to convert, ignore this situation silently. Heap will handle autoboxing.
			default: errorUnknownTag(insn->subop);
		}
		return directCall(insn->ARG0, regs[insn->ARG1], sig, insn); // return here again
	}
}


inline Instruction* Interpreter::handleHCF() {

#ifdef TRACE
	cout << "HCF" << endl;
#endif

	cerr << "VIRTUAL MACHINE ON FIRE!" << endl;
	abort();
}


inline Instruction* Interpreter::handleHLT() {

#ifdef TRACE
	cout << "HLT" << endl;
#endif

	throw ExitException();
}




// ------------- Top-level interpreter code -----------------




inline Instruction* Interpreter::processInstruction(Instruction* insn) {


	/*

			   *************
			   *    ON     *
			   *           *
			   *  [=====]  *
			   *   |   |   *
			   *   |   |   *
			   *   +   +   *
			   *           *
			   *           *
			   *           *
			   *           *
			   *    OFF    *
			   *************

		+--------------------------+
		| THE BIG SWITCH (TM) No.1 |
		+--------------------------+

	*/


	switch(insn->op) {

			case OP_NOP:    return handleNOP(insn);
			case OP_LDC:    return handleLDC(insn);
			case OP_LDCT:   return handleLDCT(insn);
			case OP_LDNULL:	return handleLDNULL(insn);
			case OP_LDF:	return handleLDF(insn);
			case OP_STF:	return handleSTF(insn);
			case OP_LDMYF:	return handleLDMYF(insn);
			case OP_STMYF:	return handleSTMYF(insn);
			case OP_LDSTAT:	return handleLDSTAT(insn);
			case OP_MOV:	return handleMOV(insn);
			case OP_XCHG:	return handleXCHG(insn);
			case OP_PUSH:	return handlePUSH(insn);
			case OP_POP:	return handlePOP(insn);
			case OP_PUSHC:	return handlePUSHC(insn);
			case OP_PUSHCT:	return handlePUSHCT(insn);
			case OP_LDS:	return handleLDS(insn);
			case OP_STS:	return handleSTS(insn);
			case OP_A3REG:  return handleA3REG(insn);
			case OP_NEG:	return handleNEG(insn);
			case OP_LNOT:	return handleLNOT(insn);
			case OP_IDX:	return handleIDX(insn);
			case OP_IDXI:	return handleIDXI(insn);
			case OP_IDXW:	return handleIDXW(insn);
			case OP_IDXWI:	return handleIDXWI(insn);
			case OP_JMP:	return handleJMP(insn);
			case OP_CALL:	return handleCALL(insn);
			case OP_CALLMY:	return handleCALLMY(insn);
			case OP_NEW:	return handleNEW(insn);
			case OP_RET:	return handleRET(insn);
			case OP_RETT:	return handleRETT(insn);
			case OP_RETNULL:return handleRETNULL();
			case OP_TRY:	return handleTRY(insn);
			case OP_CATCH:	return handleCATCH(insn);
			case OP_THROW:	return handleTHROW(insn);
			case OP_THROWT:	return handleTHROWT(insn);
			case OP_FIN:	return handleFIN(insn);
			case OP_INSTOF: return handleINSTOF(insn);
			case OP_ISTYPE: return handleISTYPE(insn);
			case OP_CNVT:   return handleCNVT(insn);
			case OP_HCF:    return handleHCF();
			case OP_HLT:    return handleHLT();

			default:        return handleIllegalInstruction(insn);
	}
}



void Interpreter::start(vector<char*>& args) {

	map<string, uint16_t>::iterator mainIt = linkedTypes->find("Main");
	if(mainIt == linkedTypes->end()) {
		throw runtime_error("Main class was not found!");
	}
	uint16_t mainClassType = mainIt->second;

	#ifdef DEBUG
		cout << "Initializing interpreter environment, Main class has type " << mainClassType << endl;
	#endif

	QuaClass* mainClass = getClassFromType(mainClassType);

	if(!mainClass->isStatic()) {
		throw runtime_error("Main class is not static!");
	}

    QuaValue argCnt = createInteger(args.size());
    *(--VMSP) = argCnt;
    QuaValue qargs = newRawInstance(typeCache.typeArray);
    nativeCall(qargs, (QuaSignature*)"\1initN");

	#ifdef DEBUG
		cout << "Preparing value stack, argument count for main method is: " << args.size() << endl;
	#endif
    uint32_t index = 0;
	for(vector<char*>::iterator it = args.begin(); it != args.end(); ++it) {
		QuaValue argString = stringDeserializer(*it);
        *(--VMSP) = argString;

        QuaValue indexRef = createInteger(index);
        *(--VMSP) = indexRef;

        nativeCall(qargs, (QuaSignature*)"\2_opIndexWN");
        index++;
	}

	// push args and This pointer
	*(--VMSP) = qargs;
	*(--VMSP) = newRawInstance(mainClassType);
	VMBP = VMSP;

	#ifdef DEBUG
		cout << "Value stack ready, looking up main(1) and preparing address stack" << endl;
	#endif

	QuaMethod* mainMethod = mainClass->lookupMethod((QuaSignature*)"\1main");

	// push exit handler
	*(--ASP) = QuaFrame(NULL, mainMethod, false, false);

	// get entry point
	Instruction* pc = (Instruction*)(mainMethod->code);

	#ifdef DEBUG
		cout << "Entering interpreter loop, entry point address is " << pc
			 << ", opcode 0x" << hex << setw(2) << setfill('0') << (int)pc->op << endl << dec;
	#endif

	while(1) {

		#ifdef TRACE
			cout << "# " << CURRENT_METHOD_SIG << ":\t";
		#endif
		pc = processInstruction(pc);
	}
}
