/* 
 * File:   ClassTable.cpp
 * Author: rusty
 * 
 * Created on 17. listopad 2012, 20:20
 */

#include <vector>

#include "ClassTable.h"
#include "AST.h"

#define EMPTY_ENTRY_SIZE 0
#define EMPTY_TABLE_SIZE 0


ClassTableEntry::ClassTableEntry() : defSize(EMPTY_ENTRY_SIZE) {}


void ClassTableEntry::addField(uint16_t cpNameIndex) {
    fieldIndicies.push_back(cpNameIndex);
}


void ClassTableEntry::addMethod(uint16_t cpSigIndex, uint16_t cpCodeIndex) {
    uint32_t tmp = cpSigIndex;
    tmp = (tmp << 16) | cpCodeIndex;
    methodIndicies.push_back(tmp);
}


void ClassTableEntry::writeTable(Compiler& compiler) {
    compiler.write((char*) &nameIndex, 2); // CP index of class name
    
    
    
    compiler.write(, 2); //TODO size
    compiler.write(, 2); //TODO offset
}


void ClassTableEntry::writeDef(Compiler& compiler) {
    
    //general info
    compiler.write((char*) &ancestor, 2);
    compiler.write((char*) &flags, 2);
    
    
    uint16_t tmp16; uint32_t tmp32;
    
    //fields
    tmp16 = fieldIndicies.size();
    compiler.write((char*) &tmp16, 2);
    
    for (int i = 0; i < fieldIndicies.size(); i++) {
        tmp16 = fieldIndicies[i];
        compiler.write((char*) &tmp16, 2);
    }
    
    //methods
    tmp16 = methodIndicies.size();
    compiler.write((char*) &tmp16, 2);
    
    for (int i = 0; i < methodIndicies.size(); i++) {
        tmp32 = methodIndicies[i];
        compiler.write((char*) &tmp32, 4);
    }
    
}

uint16_t ClassTableEntry::computeDefSize() {
    
}



ClassTable::ClassTable() : totalSize(EMPTY_TABLE_SIZE) {}


uint16_t ClassTable::addClass(ClassTableEntry* entry) {
    classTableEntries.push_back(entry);
}


void ClassTable::write(Compiler& compiler) {
    
}
