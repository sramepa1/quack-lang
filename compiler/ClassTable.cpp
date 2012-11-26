/* 
 * File:   ClassTable.cpp
 * Author: rusty
 * 
 * Created on 17. listopad 2012, 20:20
 */

#include <vector>

#include "ClassTable.h"
#include "AST.h"

#define EMPTY_ENTRY_SIZE 8
#define EMPTY_TABLE_SIZE 8


ClassTableEntry::ClassTableEntry() : defSize(EMPTY_ENTRY_SIZE) {}


void ClassTableEntry::addField(uint16_t cpNameIndex) {
    defSize += 2;
    
    fieldIndicies.push_back(cpNameIndex);
}


void ClassTableEntry::addMethod(uint16_t cpSigIndex, uint16_t cpCodeIndex) {
    defSize += 4;
    
    uint32_t tmp = cpSigIndex;
    tmp = (tmp << 16) | cpCodeIndex;
    methodIndicies.push_back(tmp);
}


void ClassTableEntry::writeTable(Compiler& compiler, uint32_t offset) {
    compiler.write((char*) &nameIndex, 2); // CP index of class name
    compiler.write((char*) &defSize, 2);
    compiler.write((char*) &offset, 4);
}


void ClassTableEntry::writeDef(Compiler& compiler) {
    
    //general info
    compiler.write((char*) &ancestor, 2);
    compiler.write((char*) &flags, 2);
    
    uint16_t tmp16; uint32_t tmp32;
    
    //fields
    tmp16 = fieldIndicies.size();
    compiler.write((char*) &tmp16, 2);
    
    for (unsigned int i = 0; i < fieldIndicies.size(); i++) {
        tmp16 = fieldIndicies[i];
        compiler.write((char*) &tmp16, 2);
    }
    
    //methods
    tmp16 = methodIndicies.size();
    compiler.write((char*) &tmp16, 2);
    
    for (unsigned int i = 0; i < methodIndicies.size(); i++) {
        tmp32 = methodIndicies[i];
        compiler.write((char*) &tmp32, 4);
    }
    
    compiler.writeAlign8();
}


ClassTable::ClassTable() : totalSize(EMPTY_TABLE_SIZE) {}


void ClassTable::addClass(ClassTableEntry* entry) {
    // table entry size + definition size + alignement
    totalSize += 8 +  Compiler::sizeToAlign8(entry->defSize);
    
    classTableEntries.push_back(entry);
}


void ClassTable::write(Compiler& compiler) {
    // write header
    compiler.write((char*) &totalSize, 4); // total size
    
    uint16_t classCnt = classTableEntries.size();
    compiler.write((char*) &classCnt, 2); // count of classes
    
    compiler.writeAlign8();
    
    // write table
    uint32_t offset = EMPTY_TABLE_SIZE + classTableEntries.size() * 8;
    for(std::list<ClassTableEntry*>::iterator it = classTableEntries.begin(); it != classTableEntries.end(); ++it) {
        (*it)->writeTable(compiler, offset);
        offset += Compiler::sizeToAlign8((*it)->defSize);
        
    }
    
    // write definition
    for(std::list<ClassTableEntry*>::iterator it = classTableEntries.begin(); it != classTableEntries.end(); ++it) {
        (*it)->writeDef(compiler);
    }
}
