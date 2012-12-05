
#include "AST.h"

#include "bytecode.h"

#include <iostream>
#include <string.h>

using namespace std;


ClassTable classTable;
ConstantPool constantPool;

ClassDefinition* currentCtEntry;


void NProgram::addClass(string* s, NClass* nclass) {
    ClassDef.insert(pair<string, NClass*>(*s, nclass));
}

NProgram::~NProgram() {
    for(map<string, NClass*>::iterator it = ClassDef.begin(); it != ClassDef.end(); it++) {
        delete (it->second);
    }
}


void NProgram::compile(Compiler& compiler) {
    analyzeTree();
    
    // write offsets
    uint32_t tmp = compiler.offset + 8;
    compiler.write((char*) &tmp, 4); // classtable file offset
    
    tmp = compiler.offset + Compiler::sizeToAlign8( classTable.totalSize ) - 4; // here was u bug - check if loading fails again
    compiler.write((char*) &tmp, 4); // constatnpool file offset
    
    generateCode(compiler);
}


void NProgram::analyzeTree() {

    // Generate classtable entries
    for(map<std::string, NClass*>::iterator it = ClassDef.begin(); it != ClassDef.end(); ++it) {
        
        currentCtEntry = new ClassDefinition();
        
        // class name
        currentCtEntry->nameIndex = constantPool.addString(it->first);
           
        // flags
        currentCtEntry->flags = it->second->flags;
        
        // inheritance
        string* ancestorName = it->second->getAncestor();
        
        if(ancestorName == NULL) {
            currentCtEntry->ancestor = classTable.classTableEntries.size();
            currentCtEntry->hasAncestor = false;
        } else {
            currentCtEntry->ancestor = constantPool.addString(*ancestorName);
            currentCtEntry->hasAncestor = true;
        }

        // fields and methods
        it->second->fillTableEntry();
        
        classTable.addClass(currentCtEntry);
    }
}


void NProgram::generateCode(Compiler& compiler) {
    classTable.write(compiler);
    constantPool.write(compiler);
}


void NClass::fillTableEntry() {
    for(std::list<ClassEntry*>::iterator it = entries->begin(); it != entries->end(); ++it) {
        (*it)->fillTableEntry();
    }
}


void NField::fillTableEntry() {
    currentCtEntry->addField(*name, flags);
}


void NMethod::fillTableEntry() {
    // signature
    int size = name->size() + 2;
    char* signature = new char[size];
    signature[0] = (char) parameterNames->size();
    name->copy(signature + 1, name->size());
    uint16_t sigIndex = constantPool.addConstant(signature, size); 

    // analyze the code
    map<string, uint16_t>* localVariables = new map<string, uint16_t>();
    findLocals(localVariables);

    // generate code
    BlockTranslator* translator = new BlockTranslator(localVariables);
    generateCode(translator);
    uint16_t codeIndex = constantPool.addCode(translator);
    
    currentCtEntry->addMethod(sigIndex, flags, codeIndex, translator->instrCount(), translator->getUsedRegisterCount()); 
}


void NMethod::generateCode(BlockTranslator* translator) {
    
    map<string, uint16_t>* locals = translator->localVariables;
    
    // Copy arguments to local registers
    int16_t bpOffset = 1;
    for(list<string*>::iterator lit = parameterNames->begin(); lit !=  parameterNames->end(); ++lit, ++bpOffset) {
        map<string, uint16_t>::iterator mit = locals->find(**lit);
        if(mit == locals->end()) {
            throw "Can not happen. Something is terribly wrong.";
        } else {
            translator->addInstruction(OP_LDS, NO_SOP, mit->second, bpOffset);
        }
    }
    
    // Saving context is not needed anymore (instructions CALL, RET, etc. do this automatically)
    block->generateCode(translator);

    // return - in case there is no at the end of method
    translator->addInstruction(OP_RETNULL, NO_SOP); // we do not optimize !!!
}

void NMethod::findLocals(map<string, uint16_t>* locals) {
    cout << "Prepared to count locals" << endl;
    
    //arguments
    for(list<string*>::iterator lit = parameterNames->begin(); lit !=  parameterNames->end(); ++lit) {
        map<string, uint16_t>::iterator mit = locals->find(**lit);
        if(mit == locals->end()) {
            locals->insert(make_pair(**lit, locals->size()));
        } else {
            throw "Two method arguments can not have the same name!";
        }
    }
    
    //locals
    block->findLocals(locals);
    
    cout << "Locals counted (" << locals->size() << ")" << endl;
}


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

uint16_t loadStatClass(std::string* name, BlockTranslator* translator) {
    uint16_t classRefRegister = translator->getFreeRegister();
    translator->addInstruction(OP_LDSTAT, NO_SOP, classRefRegister, classTable.addClass(*name), 0);
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

///////////////////////////////////////////////////////////////////////////////

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

void SThrow::generateCode(BlockTranslator* translator) {
    evaluateExpression(expression, translator);
    translator->addInstruction(OP_THROW, NO_SOP, expression->resultRegister, 0, 0);
    translator->resetRegisters();
    // TODO: throw of tagged value
}

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

void ENew::generateCode(BlockTranslator* translator) {
    prepareCall(this, parameters, translator);

    string constructorName("\0init", 5);
    constructorName.at(0) = (unsigned char)parameters->size();
    translator->addInstruction(OP_NEW, NO_SOP, resultRegister, constantPool.addString(*className),
                               constantPool.addString(constructorName));
}

void EInstanceof::generateCode(BlockTranslator *translator) {
    evaluateExpression(expression, translator);
    translator->addInstruction(OP_INSTOF, NO_SOP, resultRegister, expression->resultRegister,
                               classTable.addClass(*identifier));
    translator->resetRegisters();
}

void CString::generateCode(BlockTranslator* translator) {
    CHECK_RESULT_REGISTER(this);
    translator->addInstruction(OP_LDC, NO_SOP, resultRegister, classTable.addClass("String"),
                               constantPool.addString(*value));
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
