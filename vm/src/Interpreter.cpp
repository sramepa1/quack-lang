#include "Interpreter.h"
#include "bytecode.h"
#include "globals.h"
#include "helpers.h"

#include <sstream>
#include <iomanip>
#include <stdexcept>
#include <cstdlib>

#ifdef DEBUG
#include <iostream>
#elif defined TRACE
#include <iostream>
#endif

using namespace std;

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

    QuaClass* mainClass = resolveType(mainClassType);

    if(!mainClass->isStatic()) {
        throw runtime_error("Main class is not static!");
    }

    // push This pointer
    *(--BP) = heap->allocateNew(mainClassType, mainClass->getFieldCount());
    --SP;

    // push exit handler
    *(--ASP) = QuaFrame(NULL, false, false);

    // get entry point
    Instruction* pc = (Instruction*)(mainClass->lookupMethod((QuaSignature*)"\1main")->code);

#ifdef DEBUG
    cout << "Entering interpreter loop, entry point address is " << pc << endl;
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

            case OP_NOP:
                            #ifdef TRACE
                                cout << "NOP" << endl;
                            #endif
                            return ++insn;
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
            case OP_JMP:	return handleJMP(insn);
            case OP_JCC:	return handleJCC(insn);
            case OP_CALL:	return handleCALL(insn);
            case OP_CALLMY:	return handleCALLMY(insn);
            case OP_NEW:	return handleNEW(insn);
            case OP_RET:	return handleRET(insn);
            case OP_RETT:	return handleRETT(insn);
            case OP_RETNULL:
                            #ifdef TRACE
                                cout << "RETNULL" << endl;
                            #endif
                            return performReturn(QuaValue());
            case OP_TRY:	return handleTRY(insn);
            case OP_CATCH:	return handleCATCH(insn);
            case OP_THROW:	return handleTHROW(insn);
            case OP_THROWT:	return handleTHROWT(insn);
            case OP_FIN:	return handleFIN(insn);
            case OP_HCF:
            case OP_HLT:
                            #ifdef TRACE
                                cout << "HLT" << endl;
                            #endif
                            throw ExitException();

        default:        return handleIllegalInstruction(insn);
    }
}


// may need ASM implementation
inline Instruction* Interpreter::performCall(QuaClass* type, QuaSignature* sig) {

    QuaMethod* method = type->lookupMethod(sig);

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


inline Instruction* Interpreter::performReturn(QuaValue retVal) {

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


// INCOMPLETE!
inline Instruction* Interpreter::performThrow(QuaValue qex) {

#ifdef TRACE
    cout<<"# Throwing an exception of class "<<getClass(qex)->getName()<< " from class "<< getThisClass()->getName();
#endif

    while(1) {
        if(ASP->FRAME_TYPE == EXIT) {
            // TODO: Attempt to extract qex.what
            throw runtime_error(string("Unhandled exception of class ") + getClass(qex)->getName());
        }
        if(ASP->FRAME_TYPE == METHOD) {
            // unwind
            functionEpilogue();
            ++ASP;
            continue;
        }
        if(ASP->FRAME_TYPE == FINALLY || !instanceOf(qex, ASP->EXCEPTION_TYPE)) {
            // discard junk
            ++ASP;
            continue;
        }
        // if execution is here, the frame is a correct handler
        break;
    }

    //push qex

#ifdef TRACE
    cout << " to a handler in class " << getThisClass()->getName() << endl;
#endif

    return (Instruction*)((ASP++)->code);
}


inline Instruction* Interpreter::commonCall(uint16_t destReg, QuaValue that, uint16_t sigIndex, Instruction* retAddr) {
    QuaSignature* methSig = (QuaSignature*)getCurrentCPEntry(sigIndex);
    functionPrologue(that, retAddr, true, methSig->argCnt, destReg);
    return performCall(getClass(that), methSig);
}


inline Instruction* Interpreter::handleIllegalInstruction(Instruction* insn) {
#ifdef TRACE
    cout << "Illegal instruction!" << endl; // terminate the open line from main loop
#endif
    ostringstream os;
    os << "Illegal instruction opcode 0x" << setw(2) << setfill('0') << uppercase << hex << (int)insn->op
       << " encountered at " << insn << " in class " << getThisClass()->getName() << "." << endl;
    throw runtime_error(os.str());
}


inline void Interpreter::functionPrologue(QuaValue that,void* retAddr,bool interpreted,char argCount,uint16_t destReg) {
    *(--SP) = that;                                                                                 // sub esp, 4
    *(--ASP) = QuaFrame(retAddr, interpreted, argCount, destReg, (QuaValue*)valStackHigh - BP);     // push ebp
    BP = SP;                                                                                        // mov ebp, esp
}

inline void Interpreter::functionEpilogue() {
    SP = BP + (ASP->ARG_COUNT + 1);                 // mov esp, ebp
    BP = (QuaValue*)valStackHigh - ASP->BP_OFFSET;  // pop ebp
}


inline Instruction* Interpreter::handleLDC(Instruction* insn) {

#ifdef TRACE
    cout << "LDC r" << insn->ARG0 << ", " << insn->ARG1 << ", " << insn->ARG2 << endl;
#endif

    regs[insn->ARG0] = loadConstant(insn->ARG1, insn->ARG2);
    return ++insn;
}


inline Instruction* Interpreter::handleLDF(Instruction* insn) {

#ifdef TRACE
    cout << "LDF r" << insn->ARG0 << ", r" << insn->ARG1 << ", " << insn->ARG2 << endl;
#endif

    regs[insn->ARG0] = getFieldByName(regs[insn->ARG1], getCurrentCPEntry(insn->ARG2));
    return ++insn;
}


inline Instruction* Interpreter::handleLDSTAT(Instruction* insn) {

#ifdef TRACE
    cout << "LDSTAT r" << insn->ARG0 << ", " << insn->ARG1 << endl;
#endif

    QuaClass* statclass = resolveType(insn->ARG1);
    QuaValue instance = statclass->getInstance();
    if(instance.value == 0) {
        instance = heap->allocateNew(insn->ARG1, statclass->getFieldCount());
        statclass->setInstance(instance);
        functionPrologue(instance, insn, true, 0, insn->ARG0);
        return performCall(statclass, (QuaSignature*)"\0init");
    }
    regs[insn->ARG0] = instance;
    return ++insn;
}


inline Instruction* Interpreter::handleLDCT(Instruction* insn) {
    return ++insn;
}


inline Instruction* Interpreter::handleLDNULL(Instruction* insn) {
    return ++insn;
}


inline Instruction* Interpreter::handleSTF(Instruction* insn) {
    return ++insn;
}


inline Instruction* Interpreter::handleLDMYF(Instruction* insn) {
    return ++insn;
}


inline Instruction* Interpreter::handleSTMYF(Instruction* insn) {
    return ++insn;
}


inline Instruction* Interpreter::handleMOV(Instruction* insn) {
    return ++insn;
}


inline Instruction* Interpreter::handleXCHG(Instruction* insn) {
    return ++insn;
}


inline Instruction* Interpreter::handlePUSH(Instruction* insn) {

    return ++insn;
}


inline Instruction* Interpreter::handlePOP(Instruction* insn) {
    return ++insn;
}


inline Instruction* Interpreter::handlePUSHC(Instruction* insn) {

#ifdef TRACE
    cout << "PUSHC " << insn->ARG0 << ", " << insn->ARG1 << endl;
#endif

    *(--SP) = loadConstant(insn->ARG0, insn->ARG1);
    return ++insn;
}


inline Instruction* Interpreter::handlePUSHCT(Instruction* insn) {
    return ++insn;
}


inline Instruction* Interpreter::handleLDS(Instruction* insn) {
    return ++insn;
}


inline Instruction* Interpreter::handleSTS(Instruction* insn) {
    return ++insn;
}


inline Instruction* Interpreter::handleA3REG(Instruction* insn) {
    return ++insn;
}


inline Instruction* Interpreter::handleAREGIMM(Instruction* insn) {
    return ++insn;
}


inline Instruction* Interpreter::handleNEG(Instruction* insn) {
    return ++insn;
}


inline Instruction* Interpreter::handleLNOT(Instruction* insn) {
    return ++insn;
}


inline Instruction* Interpreter::handleIDX(Instruction* insn) {
    return ++insn;
}


inline Instruction* Interpreter::handleIDXI(Instruction* insn) {
    return ++insn;
}


inline Instruction* Interpreter::handleJMP(Instruction* insn) {
    return ++insn;
}


inline Instruction* Interpreter::handleJCC(Instruction* insn) {
    return ++insn;
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


inline Instruction* Interpreter::handleNEW(Instruction* insn) {
    return ++insn;
}


inline Instruction* Interpreter::handleRET(Instruction* insn) {
    return ++insn;
}


inline Instruction* Interpreter::handleRETT(Instruction* insn) {
    return ++insn;
}


inline Instruction* Interpreter::handleTRY(Instruction* insn) {
    return ++insn;
}


inline Instruction* Interpreter::handleCATCH(Instruction* insn) {
    return ++insn;
}


inline Instruction* Interpreter::handleTHROW(Instruction* insn) {
    return ++insn;
}


inline Instruction* Interpreter::handleTHROWT(Instruction* insn) {
    return ++insn;
}


inline Instruction* Interpreter::handleFIN(Instruction* insn) {
    return ++insn;
}


