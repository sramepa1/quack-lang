/* 
 * File:   Nodes.h
 * Author: rusty
 *
 * Created on 20. říjen 2012, 11:12
 */

#ifndef NODES_H
#define	NODES_H

#include <fstream>

class Node {
public:
    virtual ~Node() {}
    
    //virtual void analyzeTree() = 0;
    virtual void generateCode(std::ofstream&) = 0;
};

class NStatement : public Node {
public:
    virtual ~NStatement() {}
};

class NExpression : public Node {
public:
    virtual ~NExpression() {}
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


#endif	/* NODES_H */

