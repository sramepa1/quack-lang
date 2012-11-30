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

class NStatement : public Node {
public:
    virtual ~NStatement() {}
    
    virtual void generateCode(BlockTranslator* translator) = 0;
};

class NExpression : public Node {
public:
    NExpression() : registerAssigned(false) {}
    virtual ~NExpression() {}

    bool registerAssigned;
    uint16_t resultRegister;

    virtual void generateCode(BlockTranslator* translator) = 0;
};

class NValue : public NExpression {
public:
    virtual ~NValue() {}
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

class EVarible;


#endif	/* NODES_H */

