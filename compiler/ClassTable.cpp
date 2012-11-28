/* 
 * File:   ClassTable.cpp
 * Author: rusty
 * 
 * Created on 17. listopad 2012, 20:20
 */

#include <vector>

#include "AST.h"
#include "ClassTable.h"

#define EMPTY_ENTRY_SIZE 8
#define EMPTY_TABLE_SIZE 8


ClassTableEntry::ClassTableEntry() : defSize(EMPTY_ENTRY_SIZE) {}


void ClassTableEntry::addField(uint16_t cpNameIndex, uint16_t flags) {
    defSize += 4;   // TODO use sizeof?
    
    struct FieldData data;
    data.cpNameIndex = cpNameIndex;
    data.flags = flags;
    
    fieldIndicies.push_back(data);
}


void ClassTableEntry::addMethod(uint16_t cpSigIndex, uint16_t flags, uint16_t cpCodeIndex, uint16_t insnCount) {
    defSize += 8;   // TODO use sizeof?
    
    struct MethodData data;
    data.cpSigIndex = cpSigIndex;
    data.flags = flags;
    data.cpBytecodeIndex = cpCodeIndex;
    data.insnCount = insnCount;
    ;
    methodIndicies.push_back(data);
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
    
    uint16_t tmp16;
    
    //fields
    tmp16 = fieldIndicies.size();
    compiler.write((char*) &tmp16, 2);
    
    for (unsigned int i = 0; i < fieldIndicies.size(); i++) {        
        compiler.write((char*) &fieldIndicies[i], 4); // TODO use sizeof?
    }
    
    //methods
    tmp16 = methodIndicies.size();
    compiler.write((char*) &tmp16, 2);
    
    for (unsigned int i = 0; i < methodIndicies.size(); i++) {
        compiler.write((char*) &methodIndicies[i], 8); // TODO use sizeof?
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
