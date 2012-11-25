
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
     // Defined classes
    for(map<std::string, NClass*>::iterator it = ClassDef.begin(); it != ClassDef.end(); ++it) {
        
        ClassTableEntry* entry = new ClassTableEntry();
        
        // class name
        char* className = (char*) it->first.c_str(); 
        uint16_t cpIndex = constantPool.addConstant(className, it->first.size() + 1);
        
        const char* test = "abcdef";
        constantPool.addConstant((char*) test, 7);
        
        classNameIndicies.insert(make_pair(it->first, cpIndex));
        entry->nameIndex = cpIndex;
        
        // flags
        entry->flags = it->second->getFlags();
        
        // inheritance
        string* ancestorName = it->second->getAncestor();
        
        if(ancestorName == NULL) {
            entry->ancestor = 0;
        } else {
            map<std::string, uint16_t>::iterator it = classNameIndicies.find(*ancestorName);

            if(it == classNameIndicies.end()) {
                throw "Superclass does not exist.";
                // TODO error - ancestor does not exist
            }

            entry->ancestor = it->second;
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