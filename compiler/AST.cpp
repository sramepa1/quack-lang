
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
    
    //TODO zálohovat kontext
    
    
    
    //TODO obnovit kontext
    
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


void NBlock::generateCode(BlockTranslator* translator) {    
    for(list<NStatement*>::iterator it = statements->begin(); it !=  statements->end(); ++it) {
        (*it)->generateCode(translator);
    }
}

void SReturn::generateCode(BlockTranslator* translator) {
    if(expression == NULL) {
        translator->addInstruction(OP_RETNULL, OP_NOP);
    } else {
        //TODO
    }
}

void SAssignment::generateCode(BlockTranslator* translator) {

    if(!expression->registerAssigned) {
        expression->registerAssigned = true;
        expression->resultRegister = translator->getFreeRegister();
    }
    expression->generateCode(translator);

    
    //TODO refactor to polymorfic calls
    /*
    if(variable->local) {
        translator->addInstruction(OP_MOV, 0, variable->resultRegister, expression->resultRegister, 0);
    } else {
        // TODO: assignment to field
    }
     */

}


/*
void generateCode(BlockTranslator* translator) {
    
}

*/
