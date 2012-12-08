/* 
 * File:   AST_Expressions.cpp
 * Author: rusty
 * 
 * Created on 7. prosinec 2012, 17:49
 */

#include "AST_Expressions.h"

#include "bytecode.h"

#include <iostream>

using namespace std;

extern ClassTable classTable;
extern ConstantPool constantPool;
extern ClassDefinition* currentCtEntry;


void EInstanceof::generateCode(BlockTranslator *translator) {
    evaluateExpression(expression, translator);
    // TODO: add key word and node for isType
    translator->addInstruction(OP_ISTYPE, NO_SOP, resultRegister, expression->resultRegister, getTypeID(*identifier));
    translator->resetRegisters();
}

void ENew::generateCode(BlockTranslator* translator) {
    prepareCall(this, parameters, translator);

    string constructorName("\0init", 5);
    constructorName.at(0) = (unsigned char)parameters->size();
    translator->addInstruction(OP_NEW, NO_SOP, resultRegister, getTypeID(*className),
                               constantPool.addString(constructorName));
}


//////////////////////
// Fields

void EStaticField::generateCode(BlockTranslator* translator) {
    CHECK_RESULT_REGISTER(this);
    translator->addInstruction(OP_LDF, NO_SOP, resultRegister, loadStatClass(className, translator),
                               constantPool.addString(*fieldName));
}

void EVariableField::generateCode(BlockTranslator* translator) {
    CHECK_RESULT_REGISTER(this);
    translator->addInstruction(OP_LDF, NO_SOP, resultRegister, variableRegister, constantPool.addString(*fieldName));
}

void EThisField::generateCode(BlockTranslator* translator) {
    if(hasAncestor()) {
        uint16_t thisRegister = translator->getFreeRegister();
        translator->addInstruction(OP_LDS, NO_SOP, thisRegister, 0, 0);
        translator->addInstruction(OP_LDF, NO_SOP, resultRegister, thisRegister, constantPool.addString(*fieldName));
    } else {
        translator->addInstruction(OP_LDMYF, NO_SOP, resultRegister, currentCtEntry->fieldLookup.at(*fieldName), 0);
    }
}





//////////////////////
// Constants

void CString::generateCode(BlockTranslator* translator) {
    CHECK_RESULT_REGISTER(this);
    translator->addInstruction(OP_LDC, NO_SOP, resultRegister, getTypeID("String"),
                               constantPool.addString(value->substr(1, value->size() - 2)));
}

void CInt::generateCode(BlockTranslator* translator) {
    CHECK_RESULT_REGISTER(this);
    translator->addInstruction(OP_LDCT, SOP_TAG_INT, (uint32_t)value, resultRegister);
}

void CFloat::generateCode(BlockTranslator* translator) {
    CHECK_RESULT_REGISTER(this);

    union {
        float f;
        uint32_t i;
    } conversion;
    conversion.f = value;

    translator->addInstruction(OP_LDCT, SOP_TAG_FLOAT, conversion.i, resultRegister);
}

void CBool::generateCode(BlockTranslator* translator) {
    CHECK_RESULT_REGISTER(this);
    translator->addInstruction(OP_LDCT, SOP_TAG_BOOL, (uint32_t)value, resultRegister);
}



//////////////////////
// Binary operators

void EBOp::createInstr(BlockTranslator* translator, int subOp) {
    CHECK_RESULT_REGISTER(this)
    evaluateExpression(left, translator);
    evaluateExpression(right, translator);
    translator->addInstruction(OP_A3REG, subOp, resultRegister, left->resultRegister, right->resultRegister);
}

void EAnd::generateCode(BlockTranslator* translator) {
    createInstr(translator, SOP_LAND);
}

void EOr::generateCode(BlockTranslator* translator) {
    createInstr(translator, SOP_LOR);
}

void EAdd::generateCode(BlockTranslator* translator) {
    createInstr(translator, SOP_ADD);
}

void ESub::generateCode(BlockTranslator* translator) {
    createInstr(translator, SOP_SUB);
}

void EMul::generateCode(BlockTranslator* translator) {
    createInstr(translator, SOP_MUL);
}

void EDiv::generateCode(BlockTranslator* translator) {
    createInstr(translator, SOP_DIV);
}

void EMod::generateCode(BlockTranslator* translator) {
    createInstr(translator, SOP_MOD);
}

void EEq::generateCode(BlockTranslator* translator) {
    createInstr(translator, SOP_EQ);
}

void ENe::generateCode(BlockTranslator* translator) {
    createInstr(translator, SOP_NEQ);
}

void ELt::generateCode(BlockTranslator* translator) {
    createInstr(translator, SOP_LT);
}

void ELe::generateCode(BlockTranslator* translator) {
    createInstr(translator, SOP_LE);
}

void EGt::generateCode(BlockTranslator* translator) {
    createInstr(translator, SOP_GT);
}

void EGe::generateCode(BlockTranslator* translator) {
    createInstr(translator, SOP_GE);
}


//////////////////////
// Unary operators

void EUOp::createInstr(BlockTranslator* translator, int op) {
    CHECK_RESULT_REGISTER(this)
    evaluateExpression(expr, translator);
    translator->addInstruction(op, NO_SOP, resultRegister, expr->resultRegister, 0);
}

void ENot::generateCode(BlockTranslator* translator) {
    createInstr(translator, OP_LNOT);
}
