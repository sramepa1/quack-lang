/* 
 * File:   Compiler.cpp
 * Author: rusty
 * 
 * Created on 17. listopad 2012, 15:41
 */

#include "Compiler.h"

#include "classfile.h"

#include <cstring>
#include <iostream>

#include "AST.h"


extern int yyparse();
extern NProgram* quackProgram;


using namespace std;


Compiler::Compiler(const char* _outputFileName) : outputFileName(_outputFileName), offset(0) {}

Compiler::~Compiler() {}


void Compiler::compile() {
    
    cout << "Opening output file \"" << outputFileName << "\"" << endl;
    
    ofs.open(outputFileName, ios::out | ios::trunc | ios::binary);
    
    if(ofs.fail()) {
        throw "Sorry,I can not open the output file.";
    }
    
    cout << "Beginning of parsing Quack source code" << endl;
    
    yyparse();
    
    cout << "Parsing complete, AST created" << endl;
    
    cout << "Writing classfile intro" << endl;
    
    // write prologs
    write("#!/usr/bin/daisy\n");
    writeAlign8();
    
    char header[8];
    *((uint32_t*) header) = MAGIC_NUM; // magic {0xD, 'U', 0xC, 'K'}

    *((uint16_t*) header + 2) = 1; // version 1
    *((uint16_t*) header + 3) = 1; // version 1
        
    write(header, 8);
    
    cout << "Beginig of AST compiling" << endl;
    
    quackProgram->compile(*this);
    
    cout << "AST compiling completed" << endl;
    
    ofs.close();
}

void Compiler::write(const char* bytes, int lenght) {
    ofs.write(bytes, lenght);
    offset += lenght;
}

void Compiler::write(const char* c_str) {
    write(c_str, strlen(c_str) + 1);
}

void Compiler::write(const char byte) {
    write(&byte, 1);
}

void Compiler::writeAlign8() {
    while(offset % 8 != 0) {
        write((char) ALLIGN_BYTE_VALUE);
    }
}

uint32_t Compiler::sizeToAlign8(uint32_t size) {
    return size % 8 == 0 ? size : size + (8 - size % 8);
}
