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
    
    uint16_t defSize;
    
    uint16_t nameIndex;
    uint16_t ancestor;
    uint16_t flags;
    
    std::vector<uint16_t> fieldIndicies;
    std::vector<uint32_t> methodIndicies;
    
    void addField(uint16_t cpNameIndex);
    void addMethod(uint16_t cpSigIndex, uint16_t cpCodeIndex);
    
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
    
    
private:

    // DISABLED
    ClassTable(const ClassTable& orig) {}
};

#endif	/* CLASSTABLE_H */

