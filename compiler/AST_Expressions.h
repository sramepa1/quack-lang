/* 
 * File:   AST_Expressions.h
 * Author: rusty
 *
 * Created on 7. prosinec 2012, 17:49
 */

#ifndef AST_EXPRESSIONS_H
#define	AST_EXPRESSIONS_H

#include "AST.h"

class EBOp : public NExpression {
public:
    virtual ~EBOp() {}
    
    NExpression* left;
    NExpression* right;

    void createInstr(BlockTranslator* translator, int subOp);
    virtual void findLocals(std::map<std::string, uint16_t>* locals) {
        left->findLocals(locals);
        right->findLocals(locals);
    }
};

class EUOp : public NExpression {
public:
    virtual ~EUOp() {}
    
    NExpression* expr;

    void createInstr(BlockTranslator* translator, int op);
    virtual void findLocals(std::map<std::string, uint16_t>* locals) { expr->findLocals(locals); }
};

class EAnd : public EBOp {
public:
    virtual ~EAnd() {}
    virtual void generateCode(BlockTranslator* translator);
};

class EOr : public EBOp {
public:
    virtual ~EOr() {}
    virtual void generateCode(BlockTranslator* translator);
};

class ENot : public EUOp {
public:
    virtual ~ENot() {}
    virtual void generateCode(BlockTranslator* translator);
};

class EAdd : public EBOp {
public:
    virtual ~EAdd() {}
    virtual void generateCode(BlockTranslator* translator);
};

class ESub : public EBOp {
public:
    virtual ~ESub() {}
    virtual void generateCode(BlockTranslator* translator);
};

class EMul : public EBOp {
public:
    virtual ~EMul() {}
    virtual void generateCode(BlockTranslator* translator);
};

class EDiv : public EBOp {
public:
    virtual ~EDiv() {}
    virtual void generateCode(BlockTranslator* translator);
};

class EMod : public EBOp {
public:
    virtual ~EMod() {}
    virtual void generateCode(BlockTranslator* translator);
};

class EEq : public EBOp {
public:
    virtual ~EEq() {}
    virtual void generateCode(BlockTranslator* translator);
};

class ENe : public EBOp {
public:
    virtual ~ENe() {}
    virtual void generateCode(BlockTranslator* translator);
};

class ELt : public EBOp {
public:
    virtual ~ELt() {}
    virtual void generateCode(BlockTranslator* translator);
};

class ELe : public EBOp {
public:
    virtual ~ELe() {}
    virtual void generateCode(BlockTranslator* translator);
};

class EGt : public EBOp {
public:
    virtual ~EGt() {}
    virtual void generateCode(BlockTranslator* translator);
};

class EGe : public EBOp {
public:
    virtual ~EGe() {}
    virtual void generateCode(BlockTranslator* translator);
};

class EInstanceof : public NExpression {
public:
    EInstanceof(NVariable* _expression, std::string* _identifier) : expression(_expression), identifier(_identifier) {}
    virtual ~EInstanceof() {}
    
    NVariable* expression;
    std::string* identifier;
    
    virtual void generateCode(BlockTranslator* translator);
    virtual void findLocals(std::map<std::string, uint16_t>* locals) {
        expression->findLocals(locals);
    }
};


class CString : public NExpression {
public:
    virtual ~CString() {}
    
    std::string* value;
    
    virtual void generateCode(BlockTranslator* translator);
    virtual void findLocals(std::map<std::string, uint16_t>*) {}
};

class CInt : public NExpression {
public:
    virtual ~CInt() {}
    
    int value;
    
    virtual void generateCode(BlockTranslator* translator);
    virtual void findLocals(std::map<std::string, uint16_t>*) {}
};

class CFloat : public NExpression {
public:
    virtual ~CFloat() {}
    
    float value;
    
    virtual void generateCode(BlockTranslator* translator);
    virtual void findLocals(std::map<std::string, uint16_t>*) {}
};

class CBool : public NExpression {
public:
    virtual ~CBool() {}
    
    bool value;
    
    virtual void generateCode(BlockTranslator* translator);
    virtual void findLocals(std::map<std::string, uint16_t>*) {}
};


class ENew : public NExpression {
public:
    ENew(std::string* _className, std::list<NExpression*>* _parameters) : className(_className), parameters(_parameters) {}
    virtual ~ENew() {}
    
    std::string* className;
    std::list<NExpression*>* parameters;
    
    virtual void generateCode(BlockTranslator* translator);
    virtual void findLocals(std::map<std::string, uint16_t>* locals) {
        for(std::list<NExpression*>::iterator it = parameters->begin(); it != parameters->end(); ++it) {
            (*it)->findLocals(locals);
        }
    }
};

class EStaticField : public NVariable {
public:
    EStaticField(std::string* _className, std::string* _fieldName) : className(_className), fieldName(_fieldName) {}
    virtual ~EStaticField() {}
    
    std::string* className;
    std::string* fieldName;

    virtual void generateCode(BlockTranslator* translator);
    virtual void findLocals(std::map<std::string, uint16_t>* locals) {}
    virtual NVariableType getVarType() { return STATIC_FIELD; }
};

class EVariableField : public NVariable {
public:
    EVariableField(std::string* _variableName, std::string* _fieldName)
        : variableName(_variableName), fieldName(_fieldName) {}
    virtual ~EVariableField() {}
    
    // The instance which the field belongs to is held in this register
    // It is not the same register as resultRegister!!!
    uint16_t variableRegister;
    std::string* variableName;
    std::string* fieldName;

    virtual void generateCode(BlockTranslator* translator);
    virtual void findLocals(std::map<std::string, uint16_t>* locals) {
        std::map<std::string, uint16_t>::iterator it = locals->find(*variableName);
        if(it == locals->end()) {
            variableRegister = locals->size();
            locals->insert(std::make_pair(*variableName, locals->size()));
        } else {
            variableRegister = it->second;
        }
    }
    virtual NVariableType getVarType() { return FIELD; }
};

class EThisField : public NVariable {
public:
    EThisField(std::string* _fieldName) : fieldName(_fieldName) {}
    virtual ~EThisField() {}
    
    std::string* fieldName;

    virtual void generateCode(BlockTranslator* translator);
    virtual void findLocals(std::map<std::string, uint16_t>* locals) {}
    virtual NVariableType getVarType() { return THIS_FIELD; }
};

class ELocalVariable : public NVariable {
public:
    ELocalVariable(std::string* _variableName) : variableName(_variableName) {}
    virtual ~ELocalVariable() {}
    
    std::string* variableName;

    virtual void generateCode(BlockTranslator*) {/*empty !!!*/}
    virtual void findLocals(std::map<std::string, uint16_t>* locals) {
        registerAssigned = true;
        std::map<std::string, uint16_t>::iterator it = locals->find(*variableName);
        if(it == locals->end()) {
            resultRegister = locals->size();
            locals->insert(std::make_pair(*variableName, locals->size()));
        } else {
            resultRegister = it->second;
        }
    }
    virtual NVariableType getVarType() { return LOCAL; }
};



#endif	/* AST_EXPRESSIONS_H */

