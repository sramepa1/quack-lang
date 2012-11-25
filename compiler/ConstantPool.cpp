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


ConstantPool::ConstantPool() : totalSize(EMPTY_POOL_SIZE), nextOffset(0) {}

ConstantPool::~ConstantPool() {}


int ConstantPool::addConstant(char* content, int size) {
    
    ConstantPoolEntry* entry = new ConstantPoolEntry();
    
    entry->data = content;
    entry->size = size;
    entry->offset = nextOffset;
    
    entries.push_back(entry);
    
    totalSize += 4 + (entry->size % 8 == 0 ? entry->size : entry->size + (8 - entry->size % 8));
    
    nextOffset += (entry->size % 8 == 0 ? entry->size : entry->size + (8 - entry->size % 8));
    
    return entries.size() - 1;
}

void ConstantPool::write(Compiler& compiler) {
    // write header
    compiler.write((char*) &totalSize, 4); // total size
    
    uint16_t itemCnt = entries.size();
    compiler.write((char*) &itemCnt, 2); // count of classes
    
    compiler.writeAlign8();
    
    // write offset table
    for(std::list<ConstantPoolEntry*>::iterator it = entries.begin(); it != entries.end(); ++it) {
        compiler.write((char*) &((*it)->offset), 4);
    }
    
    compiler.writeAlign8();
    
    // write data
    for(std::list<ConstantPoolEntry*>::iterator it = entries.begin(); it != entries.end(); ++it) {
        compiler.write((char*) (*it)->data, (*it)->size);
        compiler.writeAlign8();
    }
    
}
