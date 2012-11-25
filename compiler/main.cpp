

#include "Compiler.h"

#include <iostream>

using namespace std;


int main(int argc, char** argv) {
     
    cout << "Prepairing to build Quack program." << endl; 
    
    Compiler compiler;
    
    try {
        compiler.compile();
    } catch (const char* message) {
        cerr << endl << message << endl << endl;
        cout << "Build failed!" << endl;
        return 1;
    }
        
    cout << "Build succeded!" << endl;
    
    return 0;
}

