
#include "AST.h"

#include <iostream>

using namespace std;


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

        classTable.addClass(entry);
    }
}


void NProgram::generateCode(Compiler& compiler) {
    classTable.write(compiler);
    
    constantPool.write(compiler);
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