/* 
 * File:   ConstantPool.h
 * Author: rusty
 *
 * Created on 20. listopad 2012, 21:27
 */

#ifndef CONSTANTPOOL_H
#define	CONSTANTPOOL_H

#include <list>
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
    uint16_t itemCnt;
    
    int offset;
    
    std::list<ConstantPoolEntry*>* entries;
    
    int addConstant(char* content, int size);
    
private:

    // DISABLED
    ConstantPool(const ConstantPool& orig) {}
};

#endif	/* CONSTANTPOOL_H */

