/* 
 * File:   Nodes.h
 * Author: rusty
 *
 * Created on 20. říjen 2012, 11:12
 */

#ifndef NODES_H
#define	NODES_H

class Node {
public:
    virtual ~Node() {}
    
    virtual void generateCode() = 0;
};

class NStatement : public Node {
public:
    virtual ~NStatement() {}
};

class NExpression : public Node {
public:
    virtual ~NExpression() {}
};



#endif	/* NODES_H */

