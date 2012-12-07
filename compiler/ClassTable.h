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
    virtual ~ClassTableEntry() {}
    
    uint16_t defSize;
    uint16_t nameIndex;
    uint16_t classTableIndex;
    
    virtual void writeTable(Compiler& compiler, uint32_t offset) = 0;
    virtual void writeDef(Compiler& compiler) = 0;
};



class ClassDefinition : public ClassTableEntry {
public:
    ClassDefinition();
    virtual ~ClassDefinition() {}
    
    uint16_t ancestor;
    bool hasAncestor;
    uint16_t flags;
    
    std::vector<FieldData> fieldIndicies;
    std::vector<MethodData> methodIndicies;
    
    std::map<std::string, uint16_t> fieldLookup;
    
    uint16_t addField(std::string name, uint16_t flags = DEFAULT_FIELD_FLAGS);
    void addMethod(uint16_t cpSigIndex, uint16_t flags, uint16_t cpCodeIndex, uint16_t insnCount, uint16_t regCount);
    
    virtual void writeTable(Compiler& compiler, uint32_t offset);
    virtual void writeDef(Compiler& compiler);
    
private:

    // DISABLED
    ClassDefinition(const ClassDefinition& orig) {}
};


class ClassReference : public ClassTableEntry {
public:
    ClassReference(uint16_t nameIndex);
    virtual ~ClassReference() {}
    
    virtual void writeTable(Compiler& compiler, uint32_t offset);
    virtual void writeDef(Compiler& compiler);
};



class ClassTable {
public:
    ClassTable();
    virtual ~ClassTable() {}
    
    uint32_t totalSize;
    
    std::list<ClassTableEntry*> classTableEntries;   
    std::map<std::string, uint16_t> classLookup;
     
    void write(Compiler& compiler);
    
    // returns index of added class
    uint16_t addClass(std::string name);
    void addClass(ClassTableEntry* entry);

private:

    // DISABLED
    ClassTable(const ClassTable& orig) {}
};

#endif	/* CLASSTABLE_H */

