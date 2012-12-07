

#include "Compiler.h"

#include <iostream>

using namespace std;


int main(int argc, char** argv) {
     
    const char* outputFile;
    
    switch(argc) {
        case 1 :
            outputFile = "a.qc";
            break;
            
        case 2 :
            outputFile = argv[1];
            break;
            
        default:
            return 1;
    }
    
    cout << "Prepairing to build Quack program." << endl; 
    
    Compiler compiler(outputFile);
    
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

