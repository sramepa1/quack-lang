/*
 * File:   AST.h
 * Author: rusty
 *
 * Created on 20. říjen 2012, 11:16
 */


#ifndef AST_H
#define	AST_H

#include "BlockTranslator.h"
#include "Compiler.h"
#include "ClassTable.h"
#include "ConstantPool.h"

#include "classfile.h"

#include <iostream>
#include <list>
#include <map>
#include <set>
#include <string>


class Node {
public:
    virtual ~Node() {}
};

class ClassEntry : public Node {
public:
    ClassEntry() : name(NULL), flags(0) {}
    virtual ~ClassEntry() {}
    
    std::string* name;
    uint16_t flags;
    
    virtual void fillTableEntry() = 0;
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

enum NVariableType {
    LOCAL, FIELD, THIS_FIELD, STATIC_FIELD
};  // don't better way to do this :-(

class NVariable : public NExpression {
public:
    virtual ~NVariable() {}
    virtual NVariableType getVarType() = 0;
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



//class NMethod;
//class NBlock;
//class NStatement;
//class NCatch;



////////////////////////////////////////////////////////////////////////////////
// helpers for code generating

#define CHECK_RESULT_REGISTER(checkedExpr)                                       \
    if(!checkedExpr->registerAssigned) {                                         \
        throw "line: __LINE__ - expression does not have any register assigned!";\
    }

#define TYPE_UNRESOLVED 0x8000


void evaluateExpression(NExpression* expr, BlockTranslator* translator);

bool hasAncestor();

uint16_t getTypeID(std::string name);

uint16_t loadStatClass(std::string* name, BlockTranslator* translator);

void prepareCall(NExpression* callNode, std::list<NExpression*>* parameters, BlockTranslator* translator);



#endif	/* AST_H */
