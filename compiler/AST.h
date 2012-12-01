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
    NMethod(std::string* _name, std::list<NExpression*>* _parameters, NBlock* _block)
        : parameters(_parameters), block(_block) {name = _name;}
    NMethod(std::string* _name, std::list<NExpression*>* _parameters, NBlock* _block, uint16_t _flags)
        : parameters(_parameters), block(_block) {name = _name; flags = _flags;}
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
    virtual void findLocals(map<string, uint16_t>* locals) {
        for(std::list<NStatement*>::iterator it = statements->begin(); it != statements->end(); ++it) {
            (*it)->findLocals(locals);
        }
    }
};

class NReturn : public NStatement {
public:
    NReturn(NExpression* _expression = NULL) : expression(_expression) {}
    virtual ~NReturn() {}
    
    NExpression* expression;
    
    virtual void generateCode(BlockTranslator* translator);
    virtual void findLocals(map<string, uint16_t>* locals) {
        if(expression) {
            expression->findLocals(locals);
        }
    }
};

class SThrow : public NStatement {
public:
    SThrow(NExpression* _expression) : expression(_expression) {}
    virtual ~SThrow() {}
    
    NExpression* expression;
    
    virtual void generateCode(BlockTranslator* translator) {}
    virtual void findLocals(map<string, uint16_t>* locals) {
        expression->findLocals(locals);
    }
};

class SAssignment : public NStatement {
public:
    SAssignment(NVariable* _variable, NExpression* _expression) : variable(_variable), expression(_expression) {}
    virtual ~SAssignment() {}
    
    NVariable* variable;
    NExpression* expression;
    
    virtual void generateCode(BlockTranslator* translator);
    virtual void findLocals(map<string, uint16_t>* locals) {
        ((NExpression*)variable)->findLocals(locals);
        expression->findLocals(locals);
    }
};

class NThisCall : public NCall {
public:
    NThisCall(std::string* _methodName, std::list<NExpression*>* _parameters) : methodName(_methodName), parameters(_parameters) {}
    virtual ~NThisCall() {}
    
    std::string* methodName;
    std::list<NExpression*>* parameters;

    virtual void generateCode(BlockTranslator* translator) {}
    virtual void findLocals(map<string, uint16_t>* locals) {
        for(std::list<NExpression*>::iterator it = parameters->begin(); it != parameters->end(); ++it) {
            (*it)->findLocals(locals);
        }
    }
};

class NVariableCall : public NCall {
public:
    NVariableCall(std::string* _variableName, std::string* _methodName, std::list<NExpression*>* _parameters) : variableName(_variableName), methodName(_methodName), parameters(_parameters) {}
    virtual ~NVariableCall() {}
    
    std::string* variableName;
    std::string* methodName;
    std::list<NExpression*>* parameters;

    virtual void generateCode(BlockTranslator* translator) {}
    virtual void findLocals(map<string, uint16_t>* locals) {
        //TODO add variable to the locals - thought form the context it must be already there
        
        for(std::list<NExpression*>::iterator it = parameters->begin(); it != parameters->end(); ++it) {
            (*it)->findLocals(locals);
        }
    }
};

class NStaticCall : public NCall {
public:
    NStaticCall(std::string* _className, std::string* _methodName, std::list<NExpression*>* _parameters) : className(_className), methodName(_methodName), parameters(_parameters) {}
    virtual ~NStaticCall() {}
    
    std::string* className;
    std::string* methodName;
    std::list<NExpression*>* parameters;

    virtual void generateCode(BlockTranslator* translator) {}
    virtual void findLocals(map<string, uint16_t>* locals) {
        for(std::list<NExpression*>::iterator it = parameters->begin(); it != parameters->end(); ++it) {
            (*it)->findLocals(locals);
        }
    }
};

class SIf : public NStatement {
public:
    SIf(NExpression* _condition, NBlock* _thenBlock, NBlock* _elseBlock = NULL) : condition(_condition), thenBlock(_thenBlock), elseBlock(_elseBlock) {}
    virtual ~SIf() {}

    NExpression* condition;
    NBlock* thenBlock;
    NBlock* elseBlock;
    
    virtual void generateCode(BlockTranslator* translator) {}
    virtual void findLocals(map<string, uint16_t>* locals) {
        condition->findLocals(locals);
        thenBlock->findLocals(locals);
        elseBlock->findLocals(locals);
    }
};

class SFor : public NStatement {
public:
    SFor(NStatement* _init, NExpression* _condition, NStatement* _increment, NBlock* _body) : init(_init), condition(_condition), increment(_increment), body(_body) {}
    virtual ~SFor() {}
    
    NStatement* init;
    NExpression* condition;
    NStatement* increment;
    NBlock* body;

    virtual void generateCode(BlockTranslator* translator) {}
    virtual void findLocals(map<string, uint16_t>* locals) {
        init->findLocals(locals);
        condition->findLocals(locals);
        increment->findLocals(locals);
        body->findLocals(locals);
    }
};


class SWhile : public NStatement {
public:
    SWhile(NExpression* _condition, NBlock* _body) :condition(_condition), body(_body) {}
    virtual ~SWhile() {}
    
    NExpression* condition;
    NBlock* body;

    virtual void generateCode(BlockTranslator* translator) {}
    virtual void findLocals(map<string, uint16_t>* locals) {
        condition->findLocals(locals);
        body->findLocals(locals);
    }
};


////////////////////////////////////////

class EBOp : public NExpression {
public:
    virtual ~EBOp() {}
    
    NExpression* left;
    NExpression* right;

    virtual void findLocals(map<string, uint16_t>* locals) {
        left->findLocals(locals);
        right->findLocals(locals);
    }
};

class EUOp : public NExpression {
public:
    virtual ~EUOp() {}
    
    NExpression* expr;

    virtual void findLocals(map<string, uint16_t>* locals) { expr->findLocals(locals); }
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

class EInstanceof : public NExpression {
public:
    EInstanceof(NVariable* _expression, std::string* _identifier) : expression(_expression), identifier(_identifier) {}
    virtual ~EInstanceof() {}
    
    NVariable* expression;
    std::string* identifier;
    
    virtual void generateCode(BlockTranslator* translator) {}
    virtual void findLocals(map<string, uint16_t>* locals) {
        expression->findLocals(locals);
    }
};


class CString : public NExpression {
public:
    virtual ~CString() {}
    
    std::string* value;
    
    virtual void generateCode(BlockTranslator* translator) {}
    virtual void findLocals(map<string, uint16_t>*) {}
};

class CInt : public NExpression {
public:
    virtual ~CInt() {}
    
    int value;
    
    virtual void generateCode(BlockTranslator* translator) {}
    virtual void findLocals(map<string, uint16_t>*) {}
};

class CFloat : public NExpression {
public:
    virtual ~CFloat() {}
    
    float value;
    
    virtual void generateCode(BlockTranslator* translator) {}
    virtual void findLocals(map<string, uint16_t>*) {}
};

class CBool : public NExpression {
public:
    virtual ~CBool() {}
    
    bool value;
    
    virtual void generateCode(BlockTranslator* translator) {}
    virtual void findLocals(map<string, uint16_t>*) {}
};

class EStaticField : public NVariable {
public:
    EStaticField(std::string* _className, std::string* _fieldName) : className(_className), fieldName(_fieldName) {}
    virtual ~EStaticField() {}
    
    std::string* className;
    std::string* fieldName;

    virtual void generateCode(BlockTranslator* translator) {}
    virtual void findLocals(map<string, uint16_t>* locals) {}
};

class EVaribleField : public NVariable {
public:
    EVaribleField(std::string* _variableName, std::string* _fieldName) : variableName(_variableName), fieldName(_fieldName) {}
    virtual ~EVaribleField() {}
    
    std::string* variableName;
    std::string* fieldName;

    virtual void generateCode(BlockTranslator* translator) {}
    virtual void findLocals(map<string, uint16_t>* locals) {}
};

class EThisField : public NVariable {
public:
    EThisField(std::string* _fieldName) : fieldName(_fieldName) {}
    virtual ~EThisField() {}
    
    std::string* fieldName;

    virtual void generateCode(BlockTranslator* translator) {}
    virtual void findLocals(map<string, uint16_t>* locals) {}
};

class ELocalVarible : public NVariable {
public:
    ELocalVarible(std::string* _variableName) : variableName(_variableName) {}
    virtual ~ELocalVarible() {}
    
    std::string* variableName;

    virtual void generateCode(BlockTranslator* translator) {}
    virtual void findLocals(map<string, uint16_t>* locals) {
        registerAssigned = true;
        map<string, uint16_t>::iterator it = locals->find(*variableName);
        if(it == locals->end()) {
            resultRegister = locals->size();
            locals->insert(make_pair(*variableName, locals->size()));
        } else {
            resultRegister = it->second;
        }
    }
};


#endif	/* AST_H */
