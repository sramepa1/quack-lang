
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
    // Walk throght all classes
    for(map<std::string, NClass*>::iterator it = ClassDef.begin(); it != ClassDef.end(); ++it) {
        char* className = (char*) it->first.c_str(); 
        uint16_t cpIndex = constantPool.addConstant(className, it->first.size() + 1);
        classNameIndicies.insert(make_pair(it->first, cpIndex));
    }
    
    // Generate classtable entries
    for(map<std::string, NClass*>::iterator it = ClassDef.begin(); it != ClassDef.end(); ++it) {
        
        ClassTableEntry* entry = new ClassTableEntry();
        
        // class name
        map<std::string, uint16_t>::iterator it2 = classNameIndicies.find(it->first);
        entry->nameIndex = it2->second;
           
        // flags
        entry->flags = it->second->getFlags();
        
        // inheritance
        string* ancestorName = it->second->getAncestor();
        
        if(ancestorName == NULL) {
            entry->ancestor = 0;
        } else {
            it2 = classNameIndicies.find(*ancestorName);

            if(it2 == classNameIndicies.end()) {
                cout << it->first << *ancestorName << endl;
                
                throw "Superclass does not exist.";
                // TODO error - ancestor does not exist
            }

            entry->ancestor = it2->second;
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