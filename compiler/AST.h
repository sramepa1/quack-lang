/* 
 * File:   AST.h
 * Author: rusty
 *
 * Created on 20. říjen 2012, 11:16
 */

#include "Nodes.h"

#include <list>
#include <map>
#include <set>
#include <string>

#ifndef AST_H
#define	AST_H


enum ClassEntryType {FIELD, INIT, METHOD};

struct ClassEntry{
    ClassEntry(ClassEntryType _type, void* _entry) : type(_type), entry(_entry) {}
    
    ClassEntryType type;
    void* entry;
};

class NClass : public Node {
public:
    std::string* name;
    
    std::list<ClassEntry*>* entries;
    
    /*
    std::map<int, NInit*>* inits;
    std::map<std::string, NMethod*>* methods;
    std::set<std::string>* fields;
    */
    
};

class NProgram : public Node {
public:
    virtual ~NProgram();

    std::map<std::string, NClass*> classes;
    
    void addClass(std::string* name, NClass* nclass);
    virtual void generateCode();
};


class NDynClass : public NClass {
public:
    virtual ~NDynClass() {}
    
    std::string* superClass;
    
    virtual void generateCode();
};


class NStatClass : public NClass {
public:
    virtual ~NStatClass() {}
    
    virtual void generateCode();
};


class NInit : public Node {
public:
    virtual ~NInit() {}
    
    virtual void generateCode();
};


class NMethod : public Node {
public:
    virtual ~NMethod() {}
    
    std::list<std::string*>* parameters;
    
    virtual void generateCode();
};


#endif	/* AST_H */

