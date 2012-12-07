/* 
 * File:   AST_Structure.h
 * Author: rusty
 *
 * Created on 7. prosinec 2012, 17:52
 */

#ifndef AST_STRUCTURE_H
#define	AST_STRUCTURE_H

#include "AST_Statements.h"

class NClass;
class NDynClass;
class NStatClass;


class NProgram : public Node {
public:
    virtual ~NProgram();

    std::list<std::pair<std::string*, NClass*> > ClassDef;
    
    void addClass(std::string* name, NClass* nclass);
    
    virtual void analyzeTree();
    virtual void generateCode(Compiler& compiler);
    
    virtual void compile(Compiler&);
    
};


class NClass : public Node {
public:
    std::string* name;
    uint16_t flags;
    
    std::list<ClassEntry*>* entries;
    
    virtual std::string* getAncestor() = 0;
    
    void fillTableEntry();
    
};

class NDynClass : public NClass {
public:
    NDynClass() : ancestor(NULL) {flags = 0;}
    virtual ~NDynClass() {}
    
    std::string* ancestor;
    
    virtual std::string* getAncestor() {
        return ancestor;
    }
};

class NStatClass : public NClass {
public:
    NStatClass() {flags = CLS_STATIC;}
    virtual ~NStatClass() {}
    
    virtual std::string* getAncestor() {
        return NULL;
    }
};

class NField : public ClassEntry {
public:
    NField(std::string* _name) {name = _name;}
    NField(std::string* _name, uint16_t _flags) {name = _name; flags = _flags;}
    virtual ~NField() {}
    
    virtual void fillTableEntry();
};

class NMethod : public ClassEntry, public Scope {
public:
    NMethod(std::string* _name, std::list<std::string*>* _parameterNames, NBlock* _block)
        : parameterNames(_parameterNames), block(_block) {name = _name;}
    NMethod(std::string* _name, std::list<std::string*>* _parameterNames, NBlock* _block, uint16_t _flags)
        : parameterNames(_parameterNames), block(_block) {name = _name; flags = _flags;}
    virtual ~NMethod() {}
    
    std::list<std::string*>* parameterNames;
    NBlock* block;
    
    virtual void fillTableEntry();
    
    virtual void generateCode(BlockTranslator* translator);
    virtual void findLocals(std::map<std::string, uint16_t>* locals);

};

#endif	/* AST_STRUCTURE_H */

