#include "Interpreter.h"
#include "Exceptions.h"
#include "bytecode.h"
#include "globals.h"
#include "helpers.h"

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

void Interpreter::start() {

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

    // push This pointer
    *(--BP) = newRawInstance(mainClassType);
    --SP;

    // push exit handler
    *(--ASP) = QuaFrame(NULL, false, false);

    // get entry point
    Instruction* pc = (Instruction*)(mainClass->lookupMethod((QuaSignature*)"\1main")->code);

#ifdef DEBUG
    cout << "Entering interpreter loop, entry point address is " << pc
         << ", opcode 0x" << hex << setw(2) << setfill('0') << (int)pc->op << endl << dec;
#endif

    while(1) {

#ifdef TRACE
        cout << "# " << getThisClass()->getName() << ": ";
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
            case PART_A3REG|PART_ADD:
            case PART_A3REG|PART_SUB:
            case PART_A3REG|PART_MUL:
            case PART_A3REG|PART_DIV:
            case PART_A3REG|PART_MOD:
            case PART_A3REG|PART_EQ:
            case PART_A3REG|PART_NEQ:
            case PART_A3REG|PART_GT:
            case PART_A3REG|PART_GE:
            case PART_A3REG|PART_LT:
            case PART_A3REG|PART_LE:
            case PART_A3REG|PART_LAND:
            case PART_A3REG|PART_LOR:
                            return handleA3REG(insn);
            case PART_AREGIMM|PART_ADD:
            case PART_AREGIMM|PART_SUB:
            case PART_AREGIMM|PART_MUL:
            case PART_AREGIMM|PART_DIV:
            case PART_AREGIMM|PART_MOD:
            case PART_AREGIMM|PART_EQ:
            case PART_AREGIMM|PART_NEQ:
            case PART_AREGIMM|PART_GT:
            case PART_AREGIMM|PART_GE:
            case PART_AREGIMM|PART_LT:
            case PART_AREGIMM|PART_LE:
            case PART_AREGIMM|PART_LAND:
            case PART_AREGIMM|PART_LOR:
                            return handleAREGIMM(insn);
            case OP_NEG:	return handleNEG(insn);
            case OP_LNOT:	return handleLNOT(insn);
            case OP_IDX:	return handleIDX(insn);
            case OP_IDXI:	return handleIDXI(insn);
            case OP_CNVT:   return handleCNVT(insn);
            case OP_JMP:	return handleJMP(insn);
            case OP_JCC:	return handleJCC(insn);
            case OP_CALL:	return handleCALL(insn);
            case OP_CALLMY:	return handleCALLMY(insn);
            case OP_NEW:	return handleNEW(insn);
            case OP_RET:	return handleRET(insn);
            case OP_RETT:	return handleRETT(insn);
            case OP_RETNULL:return handleRETNULL();
            case OP_RETPOP:	return handleRETPOP(insn);
            case OP_TRY:	return handleTRY(insn);
            case OP_CATCH:	return handleCATCH(insn);
            case OP_THROW:	return handleTHROW(insn);
            case OP_THROWT:	return handleTHROWT(insn);
            case OP_FIN:	return handleFIN(insn);
            case OP_HCF:    return handleHCF();
            case OP_HLT:    return handleHLT();

            default:        return handleIllegalInstruction(insn);
    }
}


// may need ASM implementation
// if not, must not be inline to allow RET to work correctly (JITted code may run in this stack frame)
__attribute__ ((noinline)) Instruction* Interpreter::performCall(QuaClass* type, QuaSignature* sig) {

    QuaMethod* method;
    try {
        method = type->lookupMethod(sig);

    } catch(NoSuchMethodException& e) {
        return commonException(CLASS_NO_SUCH_METHOD_EXCEPTION, e.what());
    }

#ifdef TRACE
    cout << "# Calling " << getThisClass()->getName() << "::" << sig->name << '(' << (int)sig->argCnt << ')' << endl;
#endif

    switch(method->action) {
        case QuaMethod::INTERPRET:  return (Instruction*) method->code; //TODO set flag to compile
        case QuaMethod::COMPILE:    compiler->compile(method); // and fall-through
        case QuaMethod::JUMPTO:     throw runtime_error("Jumping to compiled code blobs is not yet implemented.");
        case QuaMethod::C_CALL:
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
__attribute__ ((noinline)) inline Instruction* Interpreter::performReturn(QuaValue retVal) {

    // discard adress stack junk
    while(ASP->FRAME_TYPE == EXCEPTION || ASP->FRAME_TYPE == FINALLY) {
        ++ASP;
    }
    if(ASP->FRAME_TYPE == EXIT) {
        throw ExitException(); // TODO Is this safe if code had passed through ASM?
    }

#ifdef TRACE
    cout << "# Returning from class " << getThisClass()->getName();
#endif

    // The frame is a correct return address (frameType == METHOD)
    functionEpilogue();

#ifdef TRACE
    cout << " to class " << getThisClass()->getName() << endl;
#endif

    if(ASP->INTERPRETED) {
        regs[ASP->DEST_REG] = retVal;
        return (Instruction*)((ASP++)->code);
    } else {
        ++ASP;
        // TODO where to put return value?
        throw runtime_error("Returning to machine code is not yet implemented");
    }
}


inline Instruction* Interpreter::performThrow(QuaValue& qex) {

#ifdef TRACE
    cout<<"# Throwing an exception of class "<<getClassFromValue(qex)->getName()<< " from class "<< getThisClass()->getName();
#endif

    while(1) {
        if(ASP->FRAME_TYPE == EXIT) {
            // TODO: Attempt to extract qex.what
#ifdef TRACE
            cout << "... no suitable handler found!" << endl;
#endif
            throw runtime_error(string("Unhandled exception of class ") + getClassFromValue(qex)->getName());

        } else if (ASP->FRAME_TYPE == EXCEPTION && instanceOf(qex, ASP->EXCEPTION_TYPE)) { // TODO: how to Pokemon?
            // correct handler
            ++ASP;
            break;
        }

        // ASP points to junk -> discard it
        if(ASP->FRAME_TYPE == METHOD) {
            // unwind
            functionEpilogue();
        }
        ++ASP;
    }

    // push exception
    *(--SP) = qex;

#ifdef TRACE
    cout << " to a handler in class " << getThisClass()->getName() << endl;
#endif

    return (Instruction*)((ASP++)->code);
}


inline Instruction* Interpreter::commonException(const char* className, const char* what) {

    QuaValue instance = newRawInstance(linkedTypes->at(className));                 // allocate
    *(--SP) = getClassFromType(linkedTypes->at(CLASS_STRING))->deserialize(what);   // push what
    instance = nativeCall(instance, (QuaSignature*)"\1initN");                      // construct

    return performThrow(instance);
}


inline Instruction* Interpreter::directCall(uint16_t destReg, QuaValue& that, QuaSignature* sig, Instruction* retAddr) {
    functionPrologue(that, retAddr, true, sig->argCnt, destReg);
    return performCall(getClassFromValue(that), sig);
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


inline void Interpreter::functionPrologue(QuaValue that,void* retAddr,bool interpreted,char argCount,uint16_t destReg) {
    *(--SP) = that;
    *(--ASP) = QuaFrame(retAddr, interpreted, argCount, destReg, (QuaValue*)valStackHigh - BP);     // push ebp
    BP = SP;                                                                                        // mov ebp, esp
}

inline void Interpreter::functionEpilogue() {
    SP = BP + (ASP->ARG_COUNT + 1);                 // mov esp, ebp
    BP = (QuaValue*)valStackHigh - ASP->BP_OFFSET;  // pop ebp
}

inline QuaValue Interpreter::extractTaggedValue(Instruction* insn) {
    return QuaValue(insn->IMM, getTaggedType(insn->subop), insn->subop);
}


#ifdef TRACE
char getTagMnemonic(uint8_t tag) {
    switch(tag) {
        case SOP_TAG_BOOL: return 'B';
        case SOP_TAG_INT: return 'I';
        case SOP_TAG_FLOAT: return 'F';
        default: return '?';
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
        functionPrologue(instance, insn, true, 0, insn->ARG0);
        return performCall(statclass, (QuaSignature*)"\0init");
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


inline Instruction* Interpreter::handleA3REG(Instruction* insn) {

    // TODO: Arithmetics (don't forget tagged values!)

    return ++insn;
}


inline Instruction* Interpreter::handleAREGIMM(Instruction* insn) {

    // TODO: Discuss ditching Reg-Imm arithmetics, as they have different semantics
    //                                             ( ADD is not + but +=, etc. )

    return ++insn;
}


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


inline Instruction* Interpreter::handleCNVT(Instruction* insn) {

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
        return directCall(insn->ARG0, regs[insn->ARG1], sig, insn + 1);
    }
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
    // TODO: discuss if using the last register as /dev/null is OK
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


// Fused POPA and RET to allow returning a value while restoring context
inline Instruction* Interpreter::handleRETPOP(Instruction* insn) {

#ifdef TRACE
    cout << "RETPOP r" << insn->ARG0 << ", r" << insn->ARG1 << ", r" << insn->ARG2 << endl;
#endif
    QuaValue retVal = regs[insn->ARG0];

    for(unsigned int i = insn->ARG2; i >= insn->ARG1; i--) {
        regs[i] = *(SP++);
    }

    return performReturn(retVal);
}


inline Instruction* Interpreter::handleTRY(Instruction* insn) {

#ifdef TRACE
    cout << "TRY " << (int32_t)insn->IMM << endl;
#endif

    *(--ASP) = QuaFrame(insn + 1 + (int32_t)insn->IMM, true, true);
    return ++insn;
}


inline Instruction* Interpreter::handleCATCH(Instruction* insn) {

#ifdef TRACE
    cout << "CATCH " << insn->REG << ", " << (int32_t)insn->IMM << endl;
#endif

    *(--ASP) = QuaFrame(insn + 1 + (int32_t)insn->IMM, true, resolveType(insn->REG));
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

    return (Instruction*)((ASP++)->code);
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
