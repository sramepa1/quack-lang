/* 
 * File:   ConstantPool.cpp
 * Author: rusty
 * 
 * Created on 20. listopad 2012, 21:27
 */

#include "ConstantPool.h"

using namespace std;

ConstantPool::ConstantPool() : totalSize(0), itemCnt(0), offset(0) {
    entries = new list<ConstantPoolEntry*>();
}

ConstantPool::~ConstantPool() {
    delete entries;
}

int ConstantPool::addConstant(char* content, int size) {
    
    ConstantPoolEntry* newEntry = new ConstantPoolEntry();
    
    newEntry->data = content;
    newEntry->size = size;
    newEntry->offset = offset;
    
    totalSize += size + 4; // 4 is for the offset table
    
    offset += size;
    
    entries->push_back(newEntry);
    
    return itemCnt++;
}
