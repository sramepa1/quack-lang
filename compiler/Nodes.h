/* 
 * File:   Nodes.h
 * Author: rusty
 *
 * Created on 20. říjen 2012, 11:12
 */

#ifndef NODES_H
#define	NODES_H

#include "BlockTranslator.h"
#include "ClassTable.h"
#include "Compiler.h"

#include <fstream>
#include <map>

class Node {
public:
    virtual ~Node() {}
    
    //virtual void analyzeTree() = 0;
    //virtual void generateCode(Compiler&) = 0;
};

class ClassEntry : public Node {
public:
    ClassEntry() : name(NULL), flags(0) {}
    virtual ~ClassEntry() {}
    
    std::string* name;
    uint16_t flags;
    
    virtual void fillTableEntry(ClassTableEntry* entry) = 0;
};

class Scope {
public:
    virtual ~Scope() {}

    virtual void findLocals(std::map<std::string, uint16_t>*) = 0;
    virtual void generateCode(BlockTranslator* translator) = 0;
};

class NStatement : public Node, public Scope {
public:
    virtual ~NStatement() {}
};

class NExpression : public Node, public Scope {
public:
    NExpression() : registerAssigned(false) {}
    virtual ~NExpression() {}

    bool registerAssigned;
    uint16_t resultRegister;
};

class NCall : public NExpression, public NStatement {
public:
    virtual ~NCall() {}
};

class NVariable : public NExpression {
public:
    virtual ~NVariable() {}
};

class Environment {
public:
    virtual ~Environment() {}
};

class LocalEnv : public Environment {
public:
    virtual ~LocalEnv() {}
    
};

class FieldEnv : public Environment {
public:
    virtual ~FieldEnv() {}
    
};


class NClass;
class NProgram;
class NDynClass;
class NStatClass;
class NMethod;
class NBlock;
class NStatement;
class NCatch;



#endif	/* NODES_H */

