/* 
 * File:   BlockTranslator.cpp
 * Author: rusty
 * 
 * Created on 28. listopad 2012, 17:41
 */

#include "BlockTranslator.h"

using namespace std;

BlockTranslator::BlockTranslator(std::map<std::string,uint16_t>* localVariables, std::map<std::string,uint16_t>* arguments) {
    this->initialRegister = localVariables->size();
    firstUnusedRegister = localVariables->size();
    
    this->localVariables = localVariables;
    this->arguments = arguments;
}

BlockTranslator::~BlockTranslator() {
    delete localVariables;
    delete arguments;
}


int BlockTranslator::addInstruction(unsigned char op, unsigned char subop, uint16_t arg0, uint16_t arg1, uint16_t arg2) {
    Instruction* instruction = new Instruction(op, subop, arg0, arg1, arg2);
    instructions.push_back(instruction);
    return instructions.size() - 1;
}


int BlockTranslator::addInstruction(unsigned char op, unsigned char subop, uint32_t imm, uint16_t reg) {
    Instruction* instruction = new Instruction(op, subop, imm, reg);
    instructions.push_back(instruction);
    return instructions.size() - 1;
}


void BlockTranslator::addTranslator(BlockTranslator* translator) {
    for(vector<Instruction*>::iterator it = translator->instructions.begin(); it != translator->instructions.end(); ++it) {
        instructions.push_back(*it);
    }
    
    delete translator;
}


int BlockTranslator::size() {
    return instructions.size() * sizeof(Instruction);
}

int BlockTranslator::instrCount() {
    return instructions.size();
}


void BlockTranslator::write(Compiler& compiler) {
    for(vector<Instruction*>::iterator it = instructions.begin(); it != instructions.end(); ++it) {
        compiler.write((char*) (*it), sizeof(Instruction));
        delete (*it);
    }
    
    instructions.clear();
}

