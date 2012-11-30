/* 
 * File:   AST.h
 * Author: rusty
 *
 * Created on 20. říjen 2012, 11:16
 */

#include "Nodes.h"
#include "ClassTable.h"
#include "ConstantPool.h"

#include "classfile.h"

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
    
    void fillTableEntry(ClassTableEntry* entry);
    
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
    
    virtual void fillTableEntry(ClassTableEntry* entry);
};

class NMethod : public ClassEntry {
public:
    NMethod(std::string* _name, std::list<NExpression*>* _parameters, NBlock* _block) : parameters(_parameters), block(_block) {name = _name;}
    NMethod(std::string* _name, std::list<NExpression*>* _parameters, NBlock* _block, uint16_t _flags) : parameters(_parameters), block(_block) {name = _name; flags = _flags;}
    virtual ~NMethod() {}
    
    std::list<NExpression*>* parameters;
    NBlock* block;
    
    virtual void fillTableEntry(ClassTableEntry* entry);
};

////////////////////////////////////////

class NBlock : public NStatement {
public:
    virtual ~NBlock() {}
    
    std::list<NStatement*>* statements;
    
    virtual void generateCode(BlockTranslator* translator);
};

class NReturn : public NStatement {
public:
    NReturn(NExpression* _expression = NULL) : expression(_expression) {}
    virtual ~NReturn() {}
    
    NExpression* expression;
    
    virtual void generateCode(BlockTranslator* translator);
};

class SAssignment : public NStatement {
public:
    virtual ~SAssignment() {}
    
    EVarible* variable;
    NExpression* expression;
    
    virtual void generateCode(BlockTranslator* translator);
};

class NCall : public NStatement, public NExpression {
public:
    virtual ~NCall() {}
    
    //TODO zařadi čí meoda se volá
    std::string* methodName;
    std::list<NExpression*>* parameters;

    virtual void generateCode(BlockTranslator* translator) {}
};

class SIf : public NStatement {
public:
    SIf() : elseBlock(NULL) {}
    virtual ~SIf() {}

    NExpression* condition;
    NBlock* thenBlock;
    NBlock* elseBlock;
    
    virtual void generateCode(BlockTranslator* translator) {}
};

class SFor : public NStatement {
public:
    virtual ~SFor() {}
    
    NStatement* init;
    NExpression* condition;
    NStatement* increment;
    NBlock* body;

    virtual void generateCode(BlockTranslator* translator) {}
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
    virtual void generateCode(BlockTranslator* translator) {}
};

class EOr : public EBOp {
public:
    virtual ~EOr() {}
    virtual void generateCode(BlockTranslator* translator) {}
};

class ENot : public EUOp {
public:
    virtual ~ENot() {}
    virtual void generateCode(BlockTranslator* translator) {}
};

class EAdd : public EBOp {
public:
    virtual ~EAdd() {}
    virtual void generateCode(BlockTranslator* translator) {}
};

class ESub : public EBOp {
public:
    virtual ~ESub() {}
    virtual void generateCode(BlockTranslator* translator) {}
};

class EMul : public EBOp {
public:
    virtual ~EMul() {}
    virtual void generateCode(BlockTranslator* translator) {}
};

class EDiv : public EBOp {
public:
    virtual ~EDiv() {}
    virtual void generateCode(BlockTranslator* translator) {}
};

class EMod : public EBOp {
public:
    virtual ~EMod() {}
    virtual void generateCode(BlockTranslator* translator) {}
};

class EEq : public EBOp {
public:
    virtual ~EEq() {}
    virtual void generateCode(BlockTranslator* translator) {}
};

class ENe : public EBOp {
public:
    virtual ~ENe() {}
    virtual void generateCode(BlockTranslator* translator) {}
};

class ELt : public EBOp {
public:
    virtual ~ELt() {}
    virtual void generateCode(BlockTranslator* translator) {}
};

class ELe : public EBOp {
public:
    virtual ~ELe() {}
    virtual void generateCode(BlockTranslator* translator) {}
};

class EGt : public EBOp {
public:
    virtual ~EGt() {}
    virtual void generateCode(BlockTranslator* translator) {}
};

class EGe : public EBOp {
public:
    virtual ~EGe() {}
    virtual void generateCode(BlockTranslator* translator) {}
};


class CString : public NExpression {
public:
    virtual ~CString() {}
    
    std::string* value;
    
    virtual void generateCode(BlockTranslator* translator) {}
};

class CInt : public NExpression {
public:
    virtual ~CInt() {}
    
    int value;
    
    virtual void generateCode(BlockTranslator* translator) {}
};

class CFloat : public NExpression {
public:
    virtual ~CFloat() {}
    
    float value;
    
    virtual void generateCode(BlockTranslator* translator) {}
};

class CBool : public NExpression {
public:
    virtual ~CBool() {}
    
    bool value;
    
    virtual void generateCode(BlockTranslator* translator) {}
};

// TODO: local vars should have a filled result register
class EVarible : public NExpression {
public:
    EVarible() : className(NULL), local(true) {}
    virtual ~EVarible() {}
    
    std::string* className;
    std::string* variableName;

    bool local;
    
    virtual void generateCode(BlockTranslator* translator) {}
};


#endif	/* AST_H */
