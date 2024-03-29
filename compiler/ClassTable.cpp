/* 
 * File:   ClassTable.cpp
 * Author: rusty
 * 
 * Created on 17. listopad 2012, 20:20
 */

#include <vector>

#include "ClassTable.h"

#include "AST.h"
#include "ConstantPool.h"

#define EMPTY_ENTRY_SIZE 8
#define EMPTY_TABLE_SIZE 8

using namespace std;

extern ConstantPool constantPool;


ClassDefinition::ClassDefinition() {
    defSize = EMPTY_ENTRY_SIZE;
}


uint16_t ClassDefinition::addField(string name, uint16_t flags) {
    
    uint16_t fieldIndex;
    map<string, uint16_t>::iterator it = fieldLookup.find(name);
    
    if(it == fieldLookup.end()) { 
        
        fieldIndex = constantPool.addString(name);
        
        defSize += sizeof(FieldData);
    
        struct FieldData data;
        data.cpNameIndex = fieldIndex;
        data.flags = flags;

        fieldIndicies.push_back(data);
        fieldLookup.insert(make_pair(name, fieldLookup.size()));
        
    } else {
        fieldIndex = it->second;
    }
    
    return fieldIndex;
}


void ClassDefinition::addMethod(uint16_t cpSigIndex, uint16_t flags, uint16_t cpCodeIndex, uint16_t insnCount, uint16_t regCount) {
    defSize += sizeof(MethodData);
    
    struct MethodData data;
    data.cpSigIndex = cpSigIndex;
    data.flags = flags;
    data.cpBytecodeIndex = cpCodeIndex;
    data.insnCount = insnCount;
    data.regCount = regCount;
    
    methodIndicies.push_back(data);
}


void ClassDefinition::writeTable(Compiler& compiler, uint32_t offset) {
    compiler.write((char*) &nameIndex, 2); // CP index of class name
    compiler.write((char*) &defSize, 2);
    compiler.write((char*) &offset, 4);
}


void ClassDefinition::writeDef(Compiler& compiler) {
    
    //general info
    if(hasAncestor) {
        compiler.write((char*) &ancestor, 2);
    } else {
        compiler.write((char*) &classTableIndex, 2);
    }
    
    compiler.write((char*) &flags, 2);
    
    uint16_t tmp16;
    
    //fields
    tmp16 = fieldIndicies.size();
    compiler.write((char*) &tmp16, 2);
    
    for (unsigned int i = 0; i < fieldIndicies.size(); i++) {        
        compiler.write((char*) &fieldIndicies[i], sizeof(FieldData));
    }
    
    //methods
    tmp16 = methodIndicies.size();
    compiler.write((char*) &tmp16, 2);
    
    for (unsigned int i = 0; i < methodIndicies.size(); i++) {
        compiler.write((char*) &methodIndicies[i], sizeof(MethodData));
    }
    
    compiler.writeAlign8();
}


ClassReference::ClassReference(uint16_t nameIndex) {
    this->nameIndex = nameIndex;
    this->defSize = 0;
}

void ClassReference::writeDef(Compiler& compiler) {}

void ClassReference::writeTable(Compiler& compiler, uint32_t offset) {
    compiler.write((char*) &nameIndex, 2); // CP index of class name
    uint32_t tmp = 0;
    compiler.write((char*) &tmp, 2);
    compiler.write((char*) &tmp, 4);
}


ClassTable::ClassTable() : totalSize(EMPTY_TABLE_SIZE) {}


void ClassTable::addClass(ClassTableEntry* entry) {
    totalSize += EMPTY_ENTRY_SIZE + Compiler::sizeToAlign8(entry->defSize); // 8 is size of class table declaration entry
    
    classTableEntries.push_back(entry);
}


uint16_t ClassTable::addClass(string name) {
    
    uint16_t classIndex;
    map<string, uint16_t>::iterator it = classLookup.find(name);
    
    if(it == classLookup.end()) { 
        ClassTableEntry* entry = new ClassReference(constantPool.addString(name));
        
        classIndex = classTableEntries.size();
        addClass(entry);

        classLookup.insert(make_pair(name, classIndex));
    } else {
        classIndex = it->second;
    }
    
    return classIndex;
}


void ClassTable::write(Compiler& compiler) {
    // write header
    compiler.write((char*) &totalSize, 4); // total size
    
    uint16_t classCnt = classTableEntries.size();
    compiler.write((char*) &classCnt, 2); // count of classes
    
    compiler.writeAlign8();
    
    // write table
    uint32_t offset = EMPTY_TABLE_SIZE + classTableEntries.size() * 8; // 8 is size of table entry
    uint16_t i = 0;
    for(std::list<ClassTableEntry*>::iterator it = classTableEntries.begin(); it != classTableEntries.end(); ++it, ++i) {
        (*it)->writeTable(compiler, offset);
        offset += Compiler::sizeToAlign8((*it)->defSize);
        (*it)->classTableIndex = i;
    }
    
    // write definition
    for(std::list<ClassTableEntry*>::iterator it = classTableEntries.begin(); it != classTableEntries.end(); ++it) {
        (*it)->writeDef(compiler);
    }
}
