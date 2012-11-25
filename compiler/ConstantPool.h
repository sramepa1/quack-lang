/* 
 * File:   ConstantPool.h
 * Author: rusty
 *
 * Created on 20. listopad 2012, 21:27
 */

#ifndef CONSTANTPOOL_H
#define	CONSTANTPOOL_H

#include "Compiler.h"

#include <list>
#include <map>
#include <vector>

// types with wknown size
extern "C" {
   #include <stdint.h>
}


struct ConstantPoolEntry{
    uint32_t offset;
    int size;
    char* data;
};


class ConstantPool {
public:
    ConstantPool();
    virtual ~ConstantPool();
    
    uint32_t totalSize;
    uint32_t nextOffset;
    
    std::list<ConstantPoolEntry*> entries;
    std::map<std::string, uint16_t> stringLookup;
    
    int addConstant(char* content, int size);
    uint16_t addString(std::string str);
    
    void write(Compiler& compiler);
    
private:

    // DISABLED
    ConstantPool(const ConstantPool& orig) {}
};

#endif	/* CONSTANTPOOL_H */

