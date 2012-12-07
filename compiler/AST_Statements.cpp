/* 
 * File:   AST_Statements.cpp
 * Author: rusty
 * 
 * Created on 7. prosinec 2012, 17:51
 */

#include "AST_Statements.h"

#include "bytecode.h"

#include <iostream>

using namespace std;

extern ClassTable classTable;
extern ConstantPool constantPool;
extern ClassDefinition* currentCtEntry;


void NBlock::generateCode(BlockTranslator* translator) {    
    for(list<NStatement*>::iterator it = statements->begin(); it !=  statements->end(); ++it) {
        (*it)->generateCode(translator);
    }
}

void SReturn::generateCode(BlockTranslator* translator) {
    if(expression == NULL) {
        translator->addInstruction(OP_RETNULL, NO_SOP);
    } else {
        evaluateExpression(expression, translator);
        translator->addInstruction(OP_RET, NO_SOP, expression->resultRegister, 0, 0);
        translator->resetRegisters();
    }
    // TODO: Return of tagged value
}

void SAssignment::generateCode(BlockTranslator* translator) {

    cout << "CodeGen: assignment" << endl;
    evaluateExpression(expression, translator);

    switch(variable->getVarType()) {

        case LOCAL: {
            translator->addInstruction(OP_MOV, NO_SOP, variable->resultRegister, expression->resultRegister, 0);
            break;
        }

        case FIELD: {
            translator->addInstruction(OP_STF, NO_SOP,
                                       ((EVariableField*)variable)->variableRegister,
                                       constantPool.addString(*((EVariableField*)variable)->fieldName),
                                       expression->resultRegister);
            break;
        }

        case THIS_FIELD: {
            if(hasAncestor()) {
                uint16_t thisRegister = translator->getFreeRegister();
                translator->addInstruction(OP_LDS, NO_SOP, thisRegister, 0, 0);
                translator->addInstruction(OP_STF, NO_SOP, thisRegister,
                                           constantPool.addString(*((EVariableField*)variable)->fieldName),
                                           expression->resultRegister);

            } else {
                translator->addInstruction(OP_STMYF, NO_SOP,
                                           currentCtEntry->fieldLookup.at(*((EThisField*)variable)->fieldName),
                                           expression->resultRegister, 0);
            }
            break;
        }

        case STATIC_FIELD: {
            translator->addInstruction(OP_STF, NO_SOP,
                                       loadStatClass(((EStaticField*)variable)->className, translator),
                                       constantPool.addString(*((EStaticField*)variable)->fieldName),
                                       expression->resultRegister);
            break;
        }

        default: {
            // something is wrong :(
            throw "Unknown type of variable!";
        }
    }
    translator->resetRegisters();
}


void NThisCall::generateCode(BlockTranslator* translator) {
    prepareCall(this, parameters, translator);
    translator->addInstruction(OP_CALLMY, NO_SOP, resultRegister,
                               constantPool.addSignature(*methodName, (unsigned char)parameters->size()), 0);

    // TODO: who cleans temporary registers??? Because method call is statement but in params are expressions
    // translator->resetRegisters();
}

void NVariableCall::generateCode(BlockTranslator* translator) {
    prepareCall(this, parameters, translator);
    translator->addInstruction(OP_CALL, NO_SOP, resultRegister, variableRegister,
                               constantPool.addSignature(*methodName, (unsigned char)parameters->size()));

    // TODO: who cleans temporary registers??? Because method call is statement but in params are expressions
    // translator->resetRegisters();
}

void NStaticCall::generateCode(BlockTranslator* translator) {
    prepareCall(this, parameters, translator);
    translator->addInstruction(OP_CALL, NO_SOP, resultRegister, loadStatClass(className, translator),
                               constantPool.addSignature(*methodName, (unsigned char)parameters->size()));

    // TODO: who cleans temporary registers??? Because method call is statement but in params are expressions
    // translator->resetRegisters();
}


///////////////////////////
// Conditional structures

void SIf::generateCode(BlockTranslator* translator) {
    evaluateExpression(condition, translator);

    uint16_t condRegister = translator->getFreeRegister();
    translator->addInstruction(OP_CNVT, SOP_TAG_BOOL, condRegister, condition->resultRegister, 0);

    int jmpInstr = translator->addInstruction(OP_JMP, SOP_CC_FALSE, 0, condRegister, 0);
    translator->resetRegisters();

    thenBlock->generateCode(translator);

    if(elseBlock != NULL) {
        int endJmp = translator->addInstruction(OP_JMP, SOP_UNCONDITIONAL, 0, 0, 0);
        translator->instructions[jmpInstr]->ARG0 = (uint16_t)(endJmp - jmpInstr);
        elseBlock->generateCode(translator);
        translator->instructions[endJmp]->ARG0 = (uint16_t)(translator->instructions.size() - 1 - endJmp);
    } else {
        translator->instructions[jmpInstr]->ARG0 = (uint16_t)(translator->instructions.size() - 1 - jmpInstr);
    }
}

void SFor::generateCode(BlockTranslator* translator) {

    if(init != NULL) {
        init->generateCode(translator);
    }

    int firstInstr = translator->instructions.size();
    evaluateExpression(condition, translator);

    uint16_t condRegister = translator->getFreeRegister();
    translator->addInstruction(OP_CNVT, SOP_TAG_BOOL, condRegister, condition->resultRegister, 0);

    int jmpInstr = translator->addInstruction(OP_JMP, SOP_CC_FALSE, 0, condRegister, 0);
    translator->resetRegisters();

    body->generateCode(translator);
    if(increment != NULL) {
        increment->generateCode(translator);
    }

    int jmpBack = translator->addInstruction(OP_JMP, SOP_UNCONDITIONAL, 0, 0, 0);
    translator->instructions[jmpBack]->ARG0 = (uint16_t)(firstInstr - jmpBack - 1);
    translator->instructions[jmpInstr]->ARG0 = (uint16_t)(jmpBack - jmpInstr);

}

void SWhile::generateCode(BlockTranslator* translator) {
    SFor whileAsFor(NULL, condition, NULL, body);
    whileAsFor.generateCode(translator);
}



//////////////////////
// Exceptions

void SThrow::generateCode(BlockTranslator* translator) {
    evaluateExpression(expression, translator);
    translator->addInstruction(OP_THROW, NO_SOP, expression->resultRegister, 0, 0);
    translator->resetRegisters();
    // TODO: throw of tagged value
}

void STry::generateCode(BlockTranslator* translator) {
    vector<int> catchInstrs;

    int tryInstr = translator->addInstruction(OP_TRY, NO_SOP, 0, 0, 0);
    for(list<NCatch*>::reverse_iterator it = catches->rbegin(); it != catches->rend(); ++it) {
        NCatch* current = *it;
        uint16_t exceptionType = 0;
        if(current->className) {
            exceptionType = getTypeID(*current->className);
        } else {
            // universal exception handler
            exceptionType = getTypeID("Exception");
        }

        catchInstrs.push_back(translator->addInstruction(OP_CATCH, NO_SOP, exceptionType, 0, 0));
    }

    block->generateCode(translator);
    translator->addInstruction(OP_FIN, NO_SOP, 0, 0, 0);

    for(list<NCatch*>::iterator it = catches->begin(); it != catches->end(); ++it) {
        translator->instructions.at(catchInstrs.at(catchInstrs.size() - 1))->ARG1
                = translator->instructions.size() - 1 - tryInstr;
        catchInstrs.pop_back();
        (*it)->generateCode(translator);
    }

    translator->instructions.at(tryInstr)->ARG0 = translator->instructions.size() - 1 - tryInstr;
}

void NCatch::generateCode(BlockTranslator* translator) {
    uint16_t exRegister = translator->localVariables->at(*variable->variableName);
    translator->addInstruction(OP_POP, SOP_STACK_1, exRegister, 0, 0);

    block->generateCode(translator);
    translator->addInstruction(OP_FIN, NO_SOP, 0, 0, 0);
}
