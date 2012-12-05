/* 
 * File:   ConstantPool.h
 * Author: rusty
 *
 * Created on 20. listopad 2012, 21:27
 */

#ifndef CONSTANTPOOL_H
#define	CONSTANTPOOL_H

#include "BlockTranslator.h"
#include "Compiler.h"

#include <list>
#include <map>
#include <vector>

// types with wknown size
extern "C" {
   #include <stdint.h>
}

class BlockTranslator;

struct ConstantPoolEntry{
    uint32_t offset;
    IWritable* writable;
};

struct Signature {
    Signature(std::string _methodName, uint16_t _parameterCount) : methodName(_methodName), parameterCount(_parameterCount) {}
    
    std::string methodName;
    unsigned char parameterCount;
};

struct SignatureComp {
    bool operator()(const Signature& first, const Signature& second) const {
        if(first.parameterCount != second.parameterCount) {
            return first.parameterCount < second.parameterCount;
        }
        return first.methodName < second.methodName;
    }
};


class ByteArray : public IWritable {
public:
    ByteArray(char* _data, int _lenght) : data(_data), lenght(_lenght) {}
    virtual ~ByteArray() {delete [] data;}
    
    char* data;
    int lenght;
    
    virtual int size() {return lenght;}
    virtual void write(Compiler& compiler);
};


class ConstantPool {
public:
    ConstantPool();
    virtual ~ConstantPool();
    
    uint32_t totalSize;
    uint32_t nextOffset;
    
    std::list<ConstantPoolEntry*> entries;
    std::map<std::string, uint16_t> stringLookup;
    std::map<Signature, uint16_t, SignatureComp> signatureLookup;
    
    uint16_t addCode(BlockTranslator* translator);
    uint16_t addConstant(char* content, int size);
    uint16_t addSignature(std::string str, unsigned char parameters);
    uint16_t addString(std::string str);
    
    void write(Compiler& compiler);
    
private:
    
    uint16_t updateCounters(IWritable* writable);

    // DISABLED
    ConstantPool(const ConstantPool& orig) {}
};

#endif	/* CONSTANTPOOL_H */



