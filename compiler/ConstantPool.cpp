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


void ByteArray::write(Compiler& compiler) {
    compiler.write(data, lenght);
}

ConstantPool::ConstantPool() : totalSize(EMPTY_POOL_SIZE), nextOffset(0) {}
ConstantPool::~ConstantPool() {}


uint16_t ConstantPool::addCode(BlockTranslator* translator) {
    
    ConstantPoolEntry* entry = new ConstantPoolEntry();
    
    entry->writable = translator;
    entry->offset = nextOffset;
    
    entries.push_back(entry);
    return updateCounters(entry->writable);
}


uint16_t ConstantPool::addConstant(char* content, int size) {
    
    ConstantPoolEntry* entry = new ConstantPoolEntry();
       
    entry->writable = new ByteArray(content, size);
    entry->offset = nextOffset;
    
    entries.push_back(entry);
    return updateCounters(entry->writable);
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
    compiler.write((char*) &totalSize, sizeof(totalSize)); // total size
    
    uint16_t itemCnt = entries.size();
    compiler.write((char*) &itemCnt, sizeof(itemCnt)); // count of classes
    
    compiler.writeAlign8();
    
    // write offset table
    uint32_t offset = EMPTY_POOL_SIZE + Compiler::sizeToAlign8(entries.size() * 4);
    for(std::list<ConstantPoolEntry*>::iterator it = entries.begin(); it != entries.end(); ++it) {
        uint32_t tmp = (*it)->offset + offset;
        compiler.write((char*) &tmp, sizeof(tmp));
    }
    
    compiler.writeAlign8();
    
    // write data
    for(std::list<ConstantPoolEntry*>::iterator it = entries.begin(); it != entries.end(); ++it) {
        (*it)->writable->write(compiler);
        compiler.writeAlign8();
        
        delete (*it);
    }
}


uint16_t ConstantPool::updateCounters(IWritable* writable) {
    totalSize += sizeof(uint32_t) + Compiler::sizeToAlign8(writable->size());  
    nextOffset += Compiler::sizeToAlign8(writable->size());
    return (uint16_t) entries.size() - 1;
}
