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


Compiler::Compiler() : outputFileName("a.qc"), offset(0) {}

Compiler::~Compiler() {}


void Compiler::compile() {
    
    ofs.open(outputFileName, ios::out | ios::trunc | ios::binary);
    
    yyparse();
    
    // write prologs
    write("#!/usr/bin/daisy\n");
    writeAlign8();
    
    char header[8];
    *((uint32_t*) header) = MAGIC_NUM; // magic {0xD, 'U', 0xC, 'K'}

    *((uint16_t*) header + 2) = 1; // version 1
    *((uint16_t*) header + 3) = 1; // version 1
        
    write(header, 8);
    
    quackProgram->compile(*this);
    
    ofs.close();
}

/*

void Compiler::writeClassTable() {
    
    
    uint16_t refeCnt = classTableEntries->size();
    
    uint32_t dataSize = 6 + refeCnt * 8 + 0; // TODO add definition sizes
    
    // table header
    write((char*) &dataSize, 4);
    write((char*) &refeCnt, 2);
    
    writeAlign8();
    
    // table entries
    for(unsigned int i = 0; i < classTableEntries->size(); i++) {
        write((char*) &classTableEntries->at(i), 8);
    }

    
    
}


*/


void Compiler::write(const char* bytes, int lenght) {
    ofs.write(bytes, lenght);
    offset += lenght;
}

void Compiler::write(const char* c_str) {
    write(c_str, strlen(c_str));
}

void Compiler::write(const char byte) {
    write(&byte, 1);
}

void Compiler::writeAlign8() {
    while(offset % 8 != 0) {
        write((char) BYTE_ALLIGN);
    }
}

uint32_t Compiler::sizeToAlign8(uint32_t size) {
    return size % 8 == 0 ? size : size + (8 - size % 8);
}
