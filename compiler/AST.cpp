
#include "AST.h"

#include <iostream>

using namespace std;


void NProgram::addClass(string* s, NClass* nclass) {
    classes.insert(pair<string, NClass*>(*s, nclass));
}

NProgram::~NProgram() {
    for(map<string, NClass*>::iterator it = classes.begin(); it != classes.end(); it++) {
        delete (it->second);
    }
}


void NProgram::generateCode(Compiler& compiler) {
    
    // Defined classes
    for(map<std::string, NClass*>::iterator it = DynClassDef.begin(); it != DynClassDef.end(); ++it) {
        
        ClassTableEntry* entry = new ClassTableEntry();
        
        // class name
        char* className = it->first.c_str(); 
        uint16_t cpIndex = constantPool.addConstant(className, it->first.size() + 1);
        
        classNameIndicies.insert(make_pair(it->first, cpIndex));
        entry->nameIndex = cpIndex;
        
        // stapic or dynamic (flags)
        //TODO 
        entry->flags = 0;
        
        if(true) {
            // ancestor
            if(it->second->ancestor == NULL) {
                entry->ancestor = 0;
            } else {
                map<std::string, uint16_t>::iterator it = classNameIndicies.find(*(entry->ancestor));

                if(it == classNameIndicies.end()) {
                    // TODO error - ancestor does not exist
                }

                entry->ancestor = it->second;
            }
        }

        
        
        
        
        
        
        classTable.addClass();
        
    }
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