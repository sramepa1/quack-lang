#include "Interpreter.h"
#include "Exceptions.h"
#include "bytecode.h"
#include "globals.h"
#include "helpers.h"

#include "StringNative.h"

#include <iostream>
#include <sstream>
#include <iomanip>
#include <stdexcept>
#include <cstdlib>

using namespace std;

#define CLASS_NO_SUCH_FIELD_EXCEPTION "NoSuchFieldException"
#define CLASS_NO_SUCH_METHOD_EXCEPTION "NoSuchMethodException"
#define CLASS_STRING "String"

#define REG_DEV_NULL 0xFFFF

Interpreter::Interpreter() : regs(vector<QuaValue>(65536)), compiler(new JITCompiler())
{
}

#ifdef TRACE
#define CURRENT_METHOD_SIG getThisClass()->getName() << "::"	\
	<< ASP->currMeth->sig->name << '(' << (int)ASP->currMeth->sig->argCnt << ')'
#endif

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

	QuaValue qargs; // TODO construct Array.

#ifdef DEBUG
	cout << "Prparing value stack, argument count for main method is: " << args.size() << endl;
#endif
	for(vector<char*>::iterator it = args.begin(); it != args.end(); ++it) {
		QuaValue argString = stringDeserializer(*it);
		//temporary
		cout << "! TODO: Would love to add String ref " << argString.value << " to args!" << endl;
	}

	// push args and This pointer
	*(--SP) = qargs;
	*(--SP) = newRawInstance(mainClassType);
	BP = SP;

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


	(this monstrosity compiles into a jump table,
	 which means very fast interpretation dispatch)

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
			case OP_JMP:	return handleJMP(insn);
			case OP_JCC:	return handleJCC(insn);
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
			case OP_HCF:    return handleHCF();
			case OP_HLT:    return handleHLT();
			case OP_INSTOF: return handleINSTOF(insn);
			case OP_ISTYPE: return handleISTYPE(insn);
			case OP_CNVT:   return handleCNVT(insn);

			default:        return handleIllegalInstruction(insn);
	}
}


// may need ASM implementation
// if not, must not be inline to allow RET to work correctly (JITted code may run in this stack frame)
__attribute__ ((noinline)) Instruction* Interpreter::performCall(QuaMethod* method) {

#ifdef TRACE
	cout << "# Calling " << CURRENT_METHOD_SIG << " [";
#endif

	switch(method->action) {
		case QuaMethod::INTERPRET:
#ifdef TRACE
			cout << "interpreting]" << endl;
#endif
			return (Instruction*) method->code; //TODO set flag to compile

		case QuaMethod::COMPILE:
#ifdef TRACE
			cout << "compiling...";
#endif

			compiler->compile(method);

#ifdef TRACE
			cout << " and ";
#endif
			// and fall-through
		case QuaMethod::JUMPTO:
#ifdef TRACE
			cout << "jumping to start]" << endl;
#endif
			throw runtime_error("Jumping to compiled code blobs is not yet implemented.");

		case QuaMethod::C_CALL:
#ifdef TRACE
			cout << "native call]" << endl;
#endif
			try {
				return performReturn( ( __extension__ (QuaValue (*)())method->code )() );
			} catch (QuaValue qex) {
				return performThrow(qex);
			}

	}
	return NULL; // to make the compiler happy...
}


// may need ASM implementation
// if not, must not be inline to allow RET to work correctly (JITted code may run in this stack frame)
__attribute__ ((noinline)) Instruction* Interpreter::performReturn(QuaValue retVal) {

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
		// TODO where to put return value?
		++ASP;

#ifdef TRACE
		cout << " to " << CURRENT_METHOD_SIG << " [jumping]" << endl;
#endif
		throw runtime_error("Returning to machine code is not yet implemented");
	}
}


inline Instruction* Interpreter::performThrow(QuaValue& qex) {

#ifdef TRACE
	cout << "# Throwing an exception of class " << getClassFromValue(qex)->getName()
	   << " from " << CURRENT_METHOD_SIG;
#endif

	while(1) {
		if(ASP->FRAME_TYPE == EXIT) {
			// TODO: Attempt to extract qex.what
#ifdef TRACE
			cout << " ...no suitable handler found!" << endl;
#endif
			throw runtime_error(string("Unhandled exception of class ") + getClassFromValue(qex)->getName());

		} else if (ASP->FRAME_TYPE == EXCEPTION && instanceOf(qex, ASP->EXCEPTION_TYPE)) { // TODO: how to Pokemon?
			// correct handler
			++ASP;
			break;
		}

		// ASP points to junk -> discard it
		if(ASP->FRAME_TYPE == METHOD) {
			// unwind value stack and restore saved context:
			functionEpilogue();
		}
		++ASP;
	}

	// push exception
	*(--SP) = qex;

#ifdef TRACE
	cout << " to a handler in " << CURRENT_METHOD_SIG << endl;
#endif

	return (Instruction*)((ASP++)->retAddr);
}


inline Instruction* Interpreter::commonException(const char* className, const char* what) {

	QuaValue instance = newRawInstance(linkedTypes->at(className));                 // allocate
	*(--SP) = getClassFromType(linkedTypes->at(CLASS_STRING))->deserialize(what);   // push what
	instance = nativeCall(instance, (QuaSignature*)"\1initN");                      // construct

	return performThrow(instance);
}


inline Instruction* Interpreter::directCall(uint16_t destReg, QuaValue& that, QuaSignature* sig, Instruction* retAddr) {
	QuaMethod* method;
	try {
		method = getClassFromValue(that)->lookupMethod(sig);

	} catch(NoSuchMethodException& e) {
		return commonException(CLASS_NO_SUCH_METHOD_EXCEPTION, e.what());
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


inline void Interpreter::functionPrologue(QuaValue that, QuaMethod* method,
										  void* retAddr, bool interpreted, uint16_t destReg) {

	// push that
	*(--SP) = that;

	// save context
	int32_t regCnt = method->regCount;
	for(int32_t i = 0; i < regCnt; i++) {
		*(--SP) = regs[i];
	}

	*(--ASP) = QuaFrame(retAddr, method, interpreted, destReg, (QuaValue*)valStackHigh - BP);		// push ebp
	BP = SP;																						// mov ebp, esp
}

inline void Interpreter::functionEpilogue() {

	int32_t regCnt = ASP->currMeth->regCount;
	SP = BP - regCnt; // discard any pushed locals and this											// mov esp, ebp

	// Restore saved context
	for(int32_t i = regCnt - 1; i >= 0; i--) {
		regs[i] = *(SP++);
	}

	SP += (ASP->ARG_COUNT + 1);											                            // add esp, N
	BP = (QuaValue*)valStackHigh - ASP->BP_OFFSET;                                                  // pop ebp
}

inline QuaValue Interpreter::extractTaggedValue(Instruction* insn) {
	return QuaValue(insn->IMM, getTaggedType(insn->subop), insn->subop);
}


#ifdef TRACE
inline char getTagMnemonic(uint8_t tag) {
	switch(tag) {
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
#endif


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

	regs[insn->ARG0] = loadConstant(insn->ARG1, insn->ARG2);
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

	QuaClass* statclass = getClassFromType(insn->ARG1); // also resolves type
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
	}

	return ++insn;
}


inline Instruction* Interpreter::handleLDMYF(Instruction* insn) {

#ifdef TRACE
	cout << "LDMYF r" << insn->ARG0 << ", " << insn->ARG1 << endl;
#endif

	regs[insn->ARG0] = getFieldByIndex(*BP, insn->ARG1);
	return ++insn;
}


inline Instruction* Interpreter::handleSTMYF(Instruction* insn) {

#ifdef TRACE
	cout << "STMYF " << insn->ARG0 << ", r" << insn->ARG1 << endl;
#endif

	getFieldByIndex(*BP, insn->ARG0) = regs[insn->ARG1];
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
			*(--SP) = regs[insn->ARG0];
			return ++insn;

		case SOP_STACK_2:
#ifdef TRACE
			cout << "PUSH2 r" << insn->ARG0 << ", r" << insn->ARG1 << endl;
#endif
			*(--SP) = regs[insn->ARG0];
			*(--SP) = regs[insn->ARG1];
			return ++insn;

		case SOP_STACK_3:
#ifdef TRACE
			cout << "PUSH3 r" << insn->ARG0 << ", r" << insn->ARG1 << ", r" << insn->ARG2 << endl;
#endif
			*(--SP) = regs[insn->ARG0];
			*(--SP) = regs[insn->ARG1];
			*(--SP) = regs[insn->ARG2];
			return ++insn;

		case SOP_STACK_RANGE:
#ifdef TRACE
			cout << "PUSHA r" << insn->ARG0 << ", r" << insn->ARG1 << endl;
#endif
			for(unsigned int i = insn->ARG0; i <= insn->ARG1; i++) {
				*(--SP) = regs[i];
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
			regs[insn->ARG0] = *(SP++);
			return ++insn;

		case SOP_STACK_2:
#ifdef TRACE
			cout << "POP2 r" << insn->ARG0 << ", r" << insn->ARG1 << endl;
#endif
			regs[insn->ARG1] = *(SP++);
			regs[insn->ARG0] = *(SP++);
			return ++insn;

		case SOP_STACK_3:
#ifdef TRACE
			cout << "POP3 r" << insn->ARG0 << ", r" << insn->ARG1 << ", r" << insn->ARG2 << endl;
#endif
			regs[insn->ARG2] = *(SP++);
			regs[insn->ARG1] = *(SP++);
			regs[insn->ARG0] = *(SP++);
			return ++insn;

		case SOP_STACK_RANGE:
#ifdef TRACE
			cout << "POPA r" << insn->ARG0 << ", r" << insn->ARG1 << endl;
#endif
			for(unsigned int i = insn->ARG1; i >= insn->ARG0; i--) {
				regs[i] = *(SP++);
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

	*(--SP) = loadConstant(insn->ARG0, insn->ARG1);
	return ++insn;
}


inline Instruction* Interpreter::handlePUSHCT(Instruction* insn) {

#ifdef TRACE
	cout << "PUSHC" << getTagMnemonic(insn->subop) << ", 0x" << hex << insn->IMM << dec << endl;
#endif

	*(--SP) = extractTaggedValue(insn);
	return ++insn;
}


inline Instruction* Interpreter::handleLDS(Instruction* insn) {

#ifdef TRACE
	cout << "LDS r" << insn->ARG0 << ", " << insn->ARG1 << endl;
#endif

	regs[insn->ARG0] = *(BP + insn->ARG1);
	return ++insn;
}


inline Instruction* Interpreter::handleSTS(Instruction* insn) {

#ifdef TRACE
	cout << "STS " << insn->ARG0 << ", r" << insn->ARG1 << endl;
#endif

	*(BP + insn->ARG0) = regs[insn->ARG1];
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


inline Instruction* Interpreter::handleIDX(Instruction* insn) {

#ifdef TRACE
	cout << "IDX r" << insn->ARG0 << ", r" << insn->ARG1 << ", r" << insn->ARG2 << endl;
#endif

	*(--SP) = regs[insn->ARG2];
	return commonIDX(insn);
}


inline Instruction* Interpreter::handleIDXI(Instruction* insn) {

#ifdef TRACE
	cout << "IDXI r" << insn->ARG0 << ", r" << insn->ARG1 << ", " << insn->ARG2 << endl;
#endif

	*(--SP) = QuaValue(insn->ARG2, typeCache.typeInteger, TAG_INT);
	return commonIDX(insn);
}


inline Instruction* Interpreter::handleJMP(Instruction* insn) {

#ifdef TRACE
	cout << "JMP " << (int32_t)insn->IMM << endl;
#endif

	return insn + 1 + (int32_t)insn->IMM;
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


inline Instruction* Interpreter::handleJCC(Instruction* insn) {

	bool condition;
	QuaValue v = regs[insn->ARG0];
	switch(insn->subop) {
		case SOP_CC_NULL: condition = isNull(v); break;
		case SOP_CC_NNULL: condition = !isNull(v); break;
		case SOP_CC_TRUE: condition = parseTaggedBool(v); break;
		case SOP_CC_FALSE: condition = !parseTaggedBool(v); break;
		default: return handleIllegalSubOp(insn);
	}
	return condition ? insn + insn->ARG2 : ++insn;
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

	return commonCall(insn->ARG0, *BP, insn->ARG1, insn + 1);
}


// TODO: make ARG2 param count to hard-coded "init" or use it as constructor signature CP index?
inline Instruction* Interpreter::handleNEW(Instruction* insn) {

#ifdef TRACE
	cout << "NEW r" << insn->ARG0 << ", " << insn->ARG1 << ", " << insn->ARG2 << endl;
#endif

	regs[insn->ARG0] = newRawInstance(insn->ARG1);
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
	cout << "TRY " << (int32_t)insn->IMM << endl;
#endif

	QuaMethod* currMeth = ASP->currMeth;
	*(--ASP) = QuaFrame(insn + 1 + (int32_t)insn->IMM, currMeth, true, true);
	return ++insn;
}


inline Instruction* Interpreter::handleCATCH(Instruction* insn) {

#ifdef TRACE
	cout << "CATCH " << insn->REG << ", " << (int32_t)insn->IMM << endl;
#endif

	QuaMethod* currMeth = ASP->currMeth;
	*(--ASP) = QuaFrame(insn + 1 + (int32_t)insn->IMM, currMeth, true, resolveType(insn->REG));
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

	regs[insn->ARG0] = createBool(instanceOf(regs[insn->ARG1], insn->ARG2));
	return ++insn;
}


inline Instruction* Interpreter::handleISTYPE(Instruction* insn) {

#ifdef TRACE
	cout << "ISTYPE r" << insn->ARG0 << ", r" << insn->ARG1 << ", "  << insn->ARG2 << endl;
#endif

	regs[insn->ARG0] = createBool(resolveType(regs[insn->ARG1].type) == resolveType(insn->ARG2));
	return ++insn;
}


inline Instruction* Interpreter::handleCNVT(Instruction* insn) {

#ifdef TRACE
	cout << "CNVT r" << insn->ARG0 << ", r" << insn->ARG1 << " -> "  << getTagMnemonic(insn->subop) << endl;
#endif

	if(regs[insn->ARG1].tag == insn->subop) {
		regs[insn->ARG0] = regs[insn->ARG1];
		return ++insn;
	} else {
		QuaSignature* sig = NULL;
		switch(insn->subop) {
			case SOP_TAG_BOOL:  sig = (QuaSignature*)"\0boolValue"; break;
			case SOP_TAG_FLOAT:  sig = (QuaSignature*)"\0floatValue"; break;
			case SOP_TAG_INT:  sig = (QuaSignature*)"\0intValue"; break;
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
