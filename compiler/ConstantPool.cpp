/* 
 * File:   ConstantPool.cpp
 * Author: rusty
 * 
 * Created on 20. listopad 2012, 21:27
 */

#include "ConstantPool.h"
#include "Compiler.h"

using namespace std;

#define EMPTY_POOL_SIZE 8


ConstantPool::ConstantPool() : totalSize(EMPTY_POOL_SIZE), nextOffset(EMPTY_POOL_SIZE) {}

ConstantPool::~ConstantPool() {}


int ConstantPool::addConstant(char* content, int size) {
    
    ConstantPoolEntry* entry = new ConstantPoolEntry();
    
    entry->data = content;
    entry->size = size;
    entry->offset = nextOffset;
    
    entries.push_back(entry);
    
    totalSize += 4 + Compiler::sizeToAlign8(entry->size);  
    nextOffset += Compiler::sizeToAlign8(entry->size);
    
    return entries.size() - 1;
}


uint16_t ConstantPool::addString(std::string str) {
    uint16_t cpIndex;
    map<std::string, uint16_t>::iterator it = stringLookup.find(str);
    
    if(it == stringLookup.end()) { 
        cpIndex = addConstant((char*) str.c_str(), str.size() + 1);
        stringLookup.insert(make_pair(str, cpIndex));
    } else {
        cpIndex = it->second;
    }
    
    return cpIndex;
}


void ConstantPool::write(Compiler& compiler) {
    // write header
    compiler.write((char*) &totalSize, 4); // total size
    
    uint16_t itemCnt = entries.size();
    compiler.write((char*) &itemCnt, 2); // count of classes
    
    compiler.writeAlign8();
    
    // write offset table
    uint32_t offset = EMPTY_POOL_SIZE + Compiler::sizeToAlign8(entries.size() * 4);
    for(std::list<ConstantPoolEntry*>::iterator it = entries.begin(); it != entries.end(); ++it) {
        compiler.write((char*) &offset, 4);
        offset += (*it)->offset;
    }
    
    compiler.writeAlign8();
    
    // write data
    for(std::list<ConstantPoolEntry*>::iterator it = entries.begin(); it != entries.end(); ++it) {
        compiler.write((char*) (*it)->data, (*it)->size);
        compiler.writeAlign8();
    }
    
}
