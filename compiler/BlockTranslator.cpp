/* 
 * File:   BlockTranslator.cpp
 * Author: rusty
 * 
 * Created on 28. listopad 2012, 17:41
 */

#include "BlockTranslator.h"

using namespace std;

BlockTranslator::BlockTranslator() {}
BlockTranslator::~BlockTranslator() {}


Instruction* BlockTranslator::addInstruction(unsigned char op, unsigned char subop, uint16_t arg0, uint16_t arg1, uint16_t arg2) {
    Instruction* instruction = new Instruction(op, subop, arg0, arg1, arg2);
    instructions.push_back(instruction);
    return instruction;
}


Instruction* BlockTranslator::addInstruction(unsigned char op, unsigned char subop, uint32_t imm, uint16_t reg) {
    Instruction* instruction = new Instruction(op, subop, imm, reg);
    instructions.push_back(instruction);
    return instruction;
}


int BlockTranslator::size() {
    return instructions.size() * sizeof(Instruction);
}

void BlockTranslator::write(Compiler& compiler) {
    for(vector<Instruction*>::iterator it = instructions.begin(); it != instructions.end(); ++it) {
        compiler.write((char*) (*it), sizeof(Instruction));
        delete (*it);
    }
    
    instructions.clear();
}

