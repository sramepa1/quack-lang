/* 
 * File:   AST_Statements.h
 * Author: rusty
 *
 * Created on 7. prosinec 2012, 17:51
 */

#ifndef AST_STATEMENTS_H
#define	AST_STATEMENTS_H

#include "AST_Expressions.h"

class NBlock : public NStatement {
public:
    virtual ~NBlock() {}
    
    std::list<NStatement*>* statements;
    
    virtual void generateCode(BlockTranslator* translator);
    virtual void findLocals(std::map<std::string, uint16_t>* locals) {
        for(std::list<NStatement*>::iterator it = statements->begin(); it != statements->end(); ++it) {
            (*it)->findLocals(locals);
        }
    }
};

class SReturn : public NStatement {
public:
    SReturn(NExpression* _expression = NULL) : expression(_expression) {}
    virtual ~SReturn() {}
    
    NExpression* expression;
    
    virtual void generateCode(BlockTranslator* translator);
    virtual void findLocals(std::map<std::string, uint16_t>* locals) {
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
    
    virtual void generateCode(BlockTranslator* translator);
    virtual void findLocals(std::map<std::string, uint16_t>* locals) {
        expression->findLocals(locals);
    }
};


class NCatch : public Node, public Scope {
public:
    NCatch(std::string* _className, std::string* _variableName, NBlock* _block)
        : className(_className), block(_block) {
             variable = new ELocalVariable(_variableName);
        }
    virtual ~NCatch() {}
    
    std::string* className;
    NBlock* block;
    ELocalVariable* variable;
    
    virtual void generateCode(BlockTranslator* translator);
    virtual void findLocals(std::map<std::string, uint16_t>* locals) {
        variable->findLocals(locals);
        block->findLocals(locals);
    }
};

class STry : public NStatement {
public:
    STry(NBlock* _block, std::list<NCatch*>* _catches) : block(_block), catches(_catches) {}
    virtual ~STry() {}
    
    NBlock* block;
    std::list<NCatch*>* catches;
    
    virtual void generateCode(BlockTranslator* translator);
    virtual void findLocals(std::map<std::string, uint16_t>* locals) {
        block->findLocals(locals);
        for(std::list<NCatch*>::iterator it = catches->begin(); it != catches->end(); ++it) {
            (*it)->findLocals(locals);
        }
    }
};

class SAssignment : public NStatement {
public:
    SAssignment(NVariable* _variable, NExpression* _expression) : variable(_variable), expression(_expression) {}
    virtual ~SAssignment() {}
    
    NVariable* variable;
    NExpression* expression;
    
    virtual void generateCode(BlockTranslator* translator);
    virtual void findLocals(std::map<std::string, uint16_t>* locals) {
        variable->findLocals(locals);
        expression->findLocals(locals);
    }
};

class NThisCall : public NCall {
public:
    NThisCall(std::string* _methodName, std::list<NExpression*>* _parameters)
        : methodName(_methodName), parameters(_parameters) {}
    virtual ~NThisCall() {}
    
    std::string* methodName;
    std::list<NExpression*>* parameters;

    virtual void generateCode(BlockTranslator* translator);
    virtual void findLocals(std::map<std::string, uint16_t>* locals) {
        for(std::list<NExpression*>::iterator it = parameters->begin(); it != parameters->end(); ++it) {
            (*it)->findLocals(locals);
        }
    }
};

class NVariableCall : public NCall {
public:
    NVariableCall(std::string* _variableName, std::string* _methodName, std::list<NExpression*>* _parameters)
        : variableName(_variableName), methodName(_methodName), parameters(_parameters) {}
    virtual ~NVariableCall() {}
    
    // The instance which the method belongs to is held in this register
    // It is not the same register as resultRegister!!!
    uint16_t variableRegister;
    std::string* variableName;
    std::string* methodName;
    std::list<NExpression*>* parameters;

    virtual void generateCode(BlockTranslator* translator);
    virtual void findLocals(std::map<std::string, uint16_t>* locals) {
        std::map<std::string, uint16_t>::iterator it = locals->find(*variableName);
        if(it == locals->end()) {
            variableRegister = locals->size();
            locals->insert(std::make_pair(*variableName, locals->size()));
        } else {
            variableRegister = it->second;
        }
        
        for(std::list<NExpression*>::iterator it = parameters->begin(); it != parameters->end(); ++it) {
            (*it)->findLocals(locals);
        }
    }
};

class NStaticCall : public NCall {
public:
    NStaticCall(std::string* _className, std::string* _methodName, std::list<NExpression*>* _parameters)
        : className(_className), methodName(_methodName), parameters(_parameters) {}
    virtual ~NStaticCall() {}
    
    std::string* className;
    std::string* methodName;
    std::list<NExpression*>* parameters;

    virtual void generateCode(BlockTranslator* translator);
    virtual void findLocals(std::map<std::string, uint16_t>* locals) {
        for(std::list<NExpression*>::iterator it = parameters->begin(); it != parameters->end(); ++it) {
            (*it)->findLocals(locals);
        }
    }
};

class SIf : public NStatement {
public:
    SIf(NExpression* _condition, NBlock* _thenBlock, NBlock* _elseBlock = NULL)
        : condition(_condition), thenBlock(_thenBlock), elseBlock(_elseBlock) {}
    virtual ~SIf() {}

    NExpression* condition;
    NBlock* thenBlock;
    NBlock* elseBlock;
    
    virtual void generateCode(BlockTranslator* translator);
    virtual void findLocals(std::map<std::string, uint16_t>* locals) {
        condition->findLocals(locals);
        thenBlock->findLocals(locals);
        if(elseBlock != NULL) {
            elseBlock->findLocals(locals);
        }
    }
};

class SFor : public NStatement {
public:
    SFor(NStatement* _init, NExpression* _condition, NStatement* _increment, NBlock* _body)
        : init(_init), condition(_condition), increment(_increment), body(_body) {}
    virtual ~SFor() {}
    
    NStatement* init;
    NExpression* condition;
    NStatement* increment;
    NBlock* body;

    virtual void generateCode(BlockTranslator* translator);
    virtual void findLocals(std::map<std::string, uint16_t>* locals) {
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

    virtual void generateCode(BlockTranslator* translator);
    virtual void findLocals(std::map<std::string, uint16_t>* locals) {
        condition->findLocals(locals);
        body->findLocals(locals);
    }
};



#endif	/* AST_STATEMENTS_H */

