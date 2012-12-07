
#include "AST.h"

#include "bytecode.h"

#include <iostream>
#include <string.h>

using namespace std;

ClassTable classTable;
ConstantPool constantPool;
ClassDefinition* currentCtEntry;


////////////////////////////////////////////////////////////////////////////////
// helpers for code generating


#define CHECK_RESULT_REGISTER(checkedExpr)                                       \
    if(!checkedExpr->registerAssigned) {                                         \
        throw "line: __LINE__ - expression does not have any register assigned!";\
    }

void evaluateExpression(NExpression* expr, BlockTranslator* translator) {
    if(!expr->registerAssigned) {
        expr->registerAssigned = true;
        expr->resultRegister = translator->getFreeRegister();
    }
    expr->generateCode(translator);
}

bool hasAncestor() {
    return currentCtEntry->hasAncestor;
}

#define TYPE_UNRESOLVED 0x8000

uint16_t getTypeID(std::string name) {
    return TYPE_UNRESOLVED | classTable.addClass(name);
}

uint16_t loadStatClass(std::string* name, BlockTranslator* translator) {
    uint16_t classRefRegister = translator->getFreeRegister();
    translator->addInstruction(OP_LDSTAT, NO_SOP, classRefRegister, getTypeID(*name), 0);
    return classRefRegister;
}

void prepareCall(NExpression* callNode, std::list<NExpression*>* parameters, BlockTranslator* translator) {
    // At first evaluate all expressions in params
    for(std::list<NExpression*>::iterator it = parameters->begin(); it != parameters->end(); ++it) {
        evaluateExpression(*it, translator);
    }

    // After that push them on stack in reverse order
    for(std::list<NExpression*>::reverse_iterator it = parameters->rbegin(); it != parameters->rend(); ++it) {
        translator->addInstruction(OP_PUSH, SOP_STACK_1, (*it)->resultRegister, 0, 0);
    }

    // If the call does not have a result register, it is necessary to assign them one
    if(!callNode->registerAssigned) {
        callNode->registerAssigned = true;
        callNode->resultRegister = translator->getFreeRegister();
    }
}

