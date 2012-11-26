
#include "AST.h"

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
        entry->flags = it->second->getFlags();
        
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
    // TODO
    
    char* code = new char[1];
    code[0] = 0x57;
    uint16_t codeIndex = constantPool.addConstant(code, 1); 
    
    entry->addMethod(sigIndex, codeIndex);
}


void NField::fillTableEntry(ClassTableEntry* entry) {
    entry->addField(constantPool.addString(*name));
}


/*

void NDynClass::generateCode(ofstream& ofs) {
    cout << "NDynClass::generateCode(ofstream& ofs)" << endl;
}

void NStatClass::generateCode(ofstream& ofs) {
    cout << "NStatClass::generateCode(ofstream& ofs)" << endl;
}

void NMethod::generateCode(ofstream& ofs) {
    cout << "NMethod::generateCode(ofstream& ofs)" << endl;
}

void NBlock::generateCode(ofstream& ofs) {
    cout << "NMethod::generateCode(ofstream& ofs)" << endl;
}
*/