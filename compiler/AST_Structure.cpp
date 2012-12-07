/* 
 * File:   AST_Structure.cpp
 * Author: rusty
 * 
 * Created on 7. prosinec 2012, 17:52
 */

#include "AST_Structure.h"

#include "bytecode.h"

#include <iostream>

using namespace std;

extern ClassTable classTable;
extern ConstantPool constantPool;
extern ClassDefinition* currentCtEntry;


void NProgram::addClass(string* s, NClass* nclass) {
    ClassDef.push_back(pair<string*, NClass*> (s, nclass));
}

NProgram::~NProgram() {
    for(std::list<std::pair<std::string*, NClass*> >::iterator it = ClassDef.begin(); it != ClassDef.end(); it++) {
        delete (it->first);
        delete (it->second);
    }
}


void NProgram::compile(Compiler& compiler) {
    analyzeTree();
    
    // write offsets
    uint32_t tmp = compiler.offset + 8;
    compiler.write((char*) &tmp, 4); // classtable file offset
    
    tmp = Compiler::sizeToAlign8(compiler.offset + classTable.totalSize + 4); // here was u bug - check if loading fails again
    compiler.write((char*) &tmp, 4); // constatnpool file offset
    
    generateCode(compiler);
}


void NProgram::analyzeTree() {

    // Generate classtable entries
    for(std::list<std::pair<std::string*, NClass*> >::iterator it = ClassDef.begin(); it != ClassDef.end(); ++it) {
        
        currentCtEntry = new ClassDefinition();
        
        // class name
        currentCtEntry->nameIndex = constantPool.addString(*it->first);
           
        // flags
        currentCtEntry->flags = it->second->flags;
        
        // inheritance
        string* ancestorName = it->second->getAncestor();
        
        if(ancestorName == NULL) {
            currentCtEntry->hasAncestor = false;
        } else {
            currentCtEntry->ancestor = classTable.addClass(*ancestorName);
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
  /*  int size = name->size() + 2;
    char* signature = new char[size];
    signature[0] = (char) parameterNames->size();
    name->copy(signature + 1, name->size());*/
    
    cout << "Generating signature for method \"" << *name << "\"" << endl;
    
    uint16_t sigIndex = constantPool.addSignature(*name, parameterNames->size());
    
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

