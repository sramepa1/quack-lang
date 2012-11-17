#include "AST.h"

#include <iostream>

using namespace std;

void NProgram::addClass(string* s, NClass* nclass) {
    classes.insert(pair<string, NClass*>(*s, nclass));
}

NProgram::~NProgram() {
    for(map<std::string, NClass*>::iterator it = classes.begin(); it != classes.end(); it++) {
        delete (it->second);
    }
}

/*
void NProgram::generateCode() {
    cout << "NProgram::generateCode()" << endl;
}

void NDynClass::generateCode() {
    cout << "NDynClass::generateCode()" << endl;
}

void NStatClass::generateCode() {
    cout << "NStatClass::generateCode()" << endl;
}

void NMethod::generateCode() {
    cout << "NMethod::generateCode()" << endl;
}

void NBlock::generateCode() {
    cout << "NMethod::generateCode()" << endl;
}
*/