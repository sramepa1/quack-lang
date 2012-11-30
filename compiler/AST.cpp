
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
    
    tmp = compiler.offset + classTable.totalSize + 4;
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


void NMethod::fillTableEntry(ClassTableEntry* entry) {
    // signature
    int size = name->size() + 2;
    char* signature = new char[size];
    signature[0] = (char) parameters->size();
    name->copy(signature + 1, name->size());
    uint16_t sigIndex = constantPool.addConstant(signature, size); 
    
    // code
    BlockTranslator* translator = new BlockTranslator();
    block->generateCode(translator);
    uint16_t codeIndex = constantPool.addCode(translator);
    
    // return - in case there is no at the end of method
    translator->addInstruction(OP_RETNULL, OP_NOP); // we do not optimize !!!
    
    entry->addMethod(sigIndex, flags, codeIndex, translator->size()); 
}


void NField::fillTableEntry(ClassTableEntry* entry) {
    entry->addField(constantPool.addString(*name), flags);
}


////////////////////////////////////////////////////////////////////////////////


void NBlock::generateCode(BlockTranslator* translator) {    
    for(list<NStatement*>::iterator it = statements->begin(); it !=  statements->end(); ++it) {
        (*it)->generateCode(translator);
    }
}

void NReturn::generateCode(BlockTranslator* translator) {
    if(expression == NULL) {
        translator->addInstruction(OP_RETNULL, OP_NOP);
    } else {
        //TODO
    }
}

void SAssignment::generateCode(BlockTranslator* translator) {
    
}


/*
void generateCode(BlockTranslator* translator) {
    
}

*/