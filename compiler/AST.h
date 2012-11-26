/* 
 * File:   AST.h
 * Author: rusty
 *
 * Created on 20. říjen 2012, 11:16
 */

#include "Nodes.h"

#include "ClassTable.h"
#include "ConstantPool.h"

#include <iostream>
#include <list>
#include <map>
#include <set>
#include <string>

#ifndef AST_H
#define	AST_H

using namespace std;


class NProgram : public Node {
public:
    virtual ~NProgram();

    std::map<std::string, NClass*> ClassDef;
    
    std::map<std::string, std::string*> DynClassRef;
    std::map<std::string, std::string*> StatClassRef;
    
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
    virtual uint16_t getFlags() = 0;
    
    void fillTableEntry(ClassTableEntry* entry);
    
};


class NDynClass : public NClass {
public:
    NDynClass() : ancestor(NULL) {}
    virtual ~NDynClass() {}
    
    std::string* ancestor;
    
    virtual std::string* getAncestor() {
        return ancestor;
    }
    
    virtual uint16_t getFlags() {
        return 0; // TODO add define
    }
    
};


class NStatClass : public NClass {
public:
    virtual ~NStatClass() {}
    
    virtual std::string* getAncestor() {
        return NULL;
    }
    
    virtual uint16_t getFlags() {
        return 1; // TODO add define
    }
    
};

class NField : public ClassEntry {
public:
    virtual ~NField() {}
    NField(std::string* _name) {name = _name;}
    
    virtual void fillTableEntry(ClassTableEntry* entry);
};

class NMethod : public ClassEntry {
public:
    virtual ~NMethod() {}
    NMethod(std::string* _name, std::list<NExpression*>* _parameters, NBlock* _block) : parameters(_parameters), block(_block) {name = _name;}
    
    std::list<NExpression*>* parameters;
    NBlock* block;
    
    virtual void fillTableEntry(ClassTableEntry* entry);
};

class NBlock : public Node {
public:
    virtual ~NBlock() {}
    
    std::list<NStatement*>* statements;
    
    virtual void generateCode(Compiler&) {}
};


////////////////////////////////////////

class EBOp : public NExpression {
public:
    virtual ~EBOp() {}
    
    NExpression* left;
    NExpression* right;
};

class EUOp : public NExpression {
public:
    virtual ~EUOp() {}
    
    NExpression* expr;
};

class EAnd : public EBOp {
public:
    virtual ~EAnd() {}
    virtual void generateCode(Compiler&) {}
};

class EOr : public EBOp {
public:
    virtual ~EOr() {}
    virtual void generateCode(Compiler&) {}
};

class ENot : public EUOp {
public:
    virtual ~ENot() {}
    virtual void generateCode(Compiler&) {}
};

class EAdd : public EBOp {
public:
    virtual ~EAdd() {}
    virtual void generateCode(Compiler&) {}
};

class ESub : public EBOp {
public:
    virtual ~ESub() {}
    virtual void generateCode(Compiler&) {}
};

class EMul : public EBOp {
public:
    virtual ~EMul() {}
    virtual void generateCode(Compiler&) {}
};

class EDiv : public EBOp {
public:
    virtual ~EDiv() {}
    virtual void generateCode(Compiler&) {}
};

class EMod : public EBOp {
public:
    virtual ~EMod() {}
    virtual void generateCode(Compiler&) {}
};

class EEq : public EBOp {
public:
    virtual ~EEq() {}
    virtual void generateCode(Compiler&) {}
};

class ENe : public EBOp {
public:
    virtual ~ENe() {}
    virtual void generateCode(Compiler&) {}
};

class ELt : public EBOp {
public:
    virtual ~ELt() {}
    virtual void generateCode(Compiler&) {}
};

class ELe : public EBOp {
public:
    virtual ~ELe() {}
    virtual void generateCode(Compiler&) {}
};

class EGt : public EBOp {
public:
    virtual ~EGt() {}
    virtual void generateCode(Compiler&) {}
};

class EGe : public EBOp {
public:
    virtual ~EGe() {}
    virtual void generateCode(Compiler&) {}
};


class CString : public NExpression {
public:
    virtual ~CString() {}
    
    std::string* value;
    
    virtual void generateCode(Compiler&) {}
};

class CInt : public NExpression {
public:
    virtual ~CInt() {}
    
    int value;
    
    virtual void generateCode(Compiler&) {}
};

class CFloat : public NExpression {
public:
    virtual ~CFloat() {}
    
    float value;
    
    virtual void generateCode(Compiler&) {}
};

class CBool : public NExpression {
public:
    virtual ~CBool() {}
    
    bool value;
    
    virtual void generateCode(Compiler&) {}
};


class EVarible : public NExpression {
public:
    EVarible() : className(NULL) {}
    virtual ~EVarible() {}
    
    std::string* className;
    std::string* variableName;
    
    virtual void generateCode(Compiler&) {}
};





////////////////////////////////////////

class SAssignment : public NStatement {
public:
    virtual ~SAssignment() {}
    
    EVarible* variable;
    NExpression* expression;
    
    virtual void generateCode(Compiler&) {}
};

class NCall : public NStatement, public NExpression {
public:
    virtual ~NCall() {}
    
    //TODO zařadi čí meoda se volá
    std::string* methodName;
    std::list<NExpression*>* parameters;

    virtual void generateCode(Compiler&) {}
};

class SIf : public NStatement {
public:
    SIf() : elseBlock(NULL) {}
    virtual ~SIf() {}

    NExpression* condition;
    NBlock* thenBlock;
    NBlock* elseBlock;
    
    virtual void generateCode(Compiler&) {}
};

class SFor : public NStatement {
public:
    virtual ~SFor() {}
    
    NStatement* init;
    NExpression* condition;
    NStatement* increment;
    NBlock* body;

    virtual void generateCode(Compiler&) {}
};




#endif	/* AST_H */
