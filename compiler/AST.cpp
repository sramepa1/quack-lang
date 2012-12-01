
#include "AST.h"

#include "bytecode.h"

#include <iostream>
#include <string.h>

using namespace std;


ClassTable classTable;
ConstantPool constantPool;


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
    
    tmp = Compiler::sizeToAlign8(compiler.offset + classTable.totalSize + 4); //TODO here was u bug - check if loading fails again
    compiler.write((char*) &tmp, 4); // constatnpool file offset
    
    generateCode(compiler);
}


void NProgram::analyzeTree() {

    // Generate classtable entries
    for(map<std::string, NClass*>::iterator it = ClassDef.begin(); it != ClassDef.end(); ++it) {
        
        ClassTableEntry* entry = new ClassTableEntry();
        
        // class name
        entry->nameIndex = constantPool.addString(it->first);
           
        // flags
        entry->flags = it->second->flags;
        
        // inheritance
        string* ancestorName = it->second->getAncestor();
        
        if(ancestorName == NULL) {
            entry->ancestor = 0;
        } else {
            entry->ancestor = constantPool.addString(*ancestorName);
        }

        // fields and methods
        it->second->fillTableEntry(entry);
        
        
        classTable.addClass(entry);
    }
}


void NProgram::generateCode(Compiler& compiler) {
    classTable.write(compiler);
    constantPool.write(compiler);
}


void NClass::fillTableEntry(ClassTableEntry* entry) {
    for(std::list<ClassEntry*>::iterator it = entries->begin(); it != entries->end(); ++it) {
        (*it)->fillTableEntry(entry);
    }
}


void NField::fillTableEntry(ClassTableEntry* entry) {
    entry->addField(constantPool.addString(*name), flags);
}


void NMethod::fillTableEntry(ClassTableEntry* entry) {
    // signature
    int size = name->size() + 2;
    char* signature = new char[size];
    signature[0] = (char) parameterNames->size();
    name->copy(signature + 1, name->size());
    uint16_t sigIndex = constantPool.addConstant(signature, size); 

    // analyze the code
    map<string, uint16_t>* localVariables = new map<string, uint16_t>();
    findLocals(localVariables);
    map<string, uint16_t>* arguments = new map<string, uint16_t>();
    findArmuments(arguments);
    
    // generate code
    BlockTranslator* translator = new BlockTranslator(localVariables, arguments);
    generateCode(translator);
    uint16_t codeIndex = constantPool.addCode(translator);
    
    entry->addMethod(sigIndex, flags, codeIndex, translator->instrCount(), translator->getUsedRegisterCount()); 
}


void NMethod::generateCode(BlockTranslator* translator) {
    
    // Saving context is not needed anymore (instructions CALL, RET, etc. do this automatically)
    block->generateCode(translator);

    // return - in case there is no at the end of method
    translator->addInstruction(OP_RETNULL, OP_NOP); // we do not optimize !!!
}

void NMethod::findLocals(map<string, uint16_t>* locals) {
    cout << "Prepared to count locals" << endl;
    block->findLocals(locals);
    cout << "Locals counted (" << locals->size() << ")" << endl;
}

void NMethod::findArmuments(map<string,uint16_t>* arguments) {
    for(list<string*>::iterator lit = parameterNames->begin(); lit !=  parameterNames->end(); ++lit) {
        map<string, uint16_t>::iterator mit = arguments->find(**lit);
        if(mit == arguments->end()) {
            arguments->insert(make_pair(**lit, arguments->size()));
        } else {
            throw "Two method arguments can not have the same name!";
        }
    }
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

uint16_t loadStatClass(std::string* name, BlockTranslator* translator) {
    uint16_t classRefRegister = translator->getFreeRegister();
    translator->addInstruction(OP_LDSTAT, NO_SOP, classRefRegister,
                               classTable.getClassIndex(constantPool.addString(*name)), 0);
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
            translator->addInstruction(OP_STMYF, NO_SOP,
                                       constantPool.addString(*((EThisField*)variable)->fieldName),
                                       expression->resultRegister, 0);
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
    // TODO: how to obtain field index from this place???
}

void NThisCall::generateCode(BlockTranslator* translator) {
    prepareCall(this, parameters, translator);
    translator->addInstruction(OP_CALLMY, NO_SOP, resultRegister, constantPool.addString(*methodName), 0);

    // TODO: who cleans temporary registers??? Because method call is statement but in params are expressions
    // translator->resetRegisters();
}

void NVariableCall::generateCode(BlockTranslator* translator) {
    prepareCall(this, parameters, translator);
    translator->addInstruction(OP_CALL, NO_SOP, resultRegister, variableRegister, constantPool.addString(*methodName));

    // TODO: who cleans temporary registers??? Because method call is statement but in params are expressions
    // translator->resetRegisters();
}

void NStaticCall::generateCode(BlockTranslator* translator) {
    prepareCall(this, parameters, translator);
    translator->addInstruction(OP_CALL, NO_SOP, resultRegister, loadStatClass(className, translator),
                               constantPool.addString(*methodName));

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
