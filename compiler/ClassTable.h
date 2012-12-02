/* 
 * File:   ClassTable.h
 * Author: rusty
 *
 * Created on 17. listopad 2012, 20:20
 */

#ifndef CLASSTABLE_H
#define	CLASSTABLE_H

#include "Compiler.h"

#include <list>
#include <map>

#define DEFAULT_FIELD_FLAGS 0

#pragma pack(1)

struct FieldData {
    uint16_t flags;
    uint16_t cpNameIndex;
};


struct MethodData {
    uint16_t flags;
    uint16_t cpSigIndex;
    uint16_t cpBytecodeIndex;
    uint16_t insnCount;
    uint16_t regCount;
};

#pragma pack()


class ClassTableEntry {
public:
    ClassTableEntry();
    virtual ~ClassTableEntry() {}
    
    uint16_t defSize;
    
    uint16_t nameIndex;
    uint16_t ancestor;
    uint16_t flags;    
    
    std::vector<FieldData> fieldIndicies;
    std::vector<MethodData> methodIndicies;
    
    std::map<std::string, uint16_t> fieldLookup;
    
    uint16_t addField(std::string name, uint16_t flags = DEFAULT_FIELD_FLAGS);
    void addMethod(uint16_t cpSigIndex, uint16_t flags, uint16_t cpCodeIndex, uint16_t insnCount, uint16_t regCount);
    
    void writeTable(Compiler& compiler, uint32_t offset);
    void writeDef(Compiler& compiler);
    
private:

    // DISABLED
    ClassTableEntry(const ClassTableEntry& orig) {}
};



class ClassTable {
public:
    ClassTable();
    virtual ~ClassTable() {}
    
    uint32_t totalSize;
    
    std::list<ClassTableEntry*> classTableEntries;
    
    // returns index of added class
    void addClass(ClassTableEntry* entry);
    
    void write(Compiler& compiler);
    
    uint16_t getClassIndex(uint16_t nameIndex);

private:

    // DISABLED
    ClassTable(const ClassTable& orig) {}
};

#endif	/* CLASSTABLE_H */

