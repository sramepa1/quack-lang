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



class ClassTableEntry{
public:
    ClassTableEntry();
    virtual ~ClassTableEntry() {}
    
    uint16_t nameIndex;
    uint16_t defSize;
    uint32_t defOffset;
    
    uint16_t ancestor;
    uint16_t flags;
    
    std::vector<uint16_t> fieldIndicies;
    std::vector<uint32_t> methodIndicies;
    
    void addField(uint16_t cpNameIndex);
    void addMethod(uint16_t cpSigIndex, uint16_t cpCodeIndex);
    
    void writeTable(Compiler& compiler);
    void writeDef(Compiler& compiler);
    
    uint16_t computeDefSize();
    
private:

    // DISABLED
    ClassTableEntry(const ClassTableEntry& orig) {}
};



class ClassTable {
public:
    ClassTable();
    virtual ~ClassTable() {}
    
    uint32_t totalSize;
    uint16_t itemCnt;
    
    std::list<ClassTableEntry*> classTableEntries;
    
    // returns index of added class
    uint16_t addClass(ClassTableEntry* entry);
    
    void write(Compiler& compiler);
    
    
private:

    // DISABLED
    ClassTable(const ClassTable& orig) {}
};

#endif	/* CLASSTABLE_H */

