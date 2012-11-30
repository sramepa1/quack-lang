/* 
 * File:   BlockTranslator.h
 * Author: rusty
 *
 * Created on 28. listopad 2012, 17:41
 */

#ifndef BLOCKTRANSLATOR_H
#define	BLOCKTRANSLATOR_H

#include "Compiler.h"
#include "ConstantPool.h"
#include "Instruction.h"

#include <vector>

class BlockTranslator : public IWritable{
public:
    BlockTranslator();
    virtual ~BlockTranslator();
    
    std::vector<Instruction*> instructions;
    
    int addInstruction(unsigned char op,
                unsigned char subop,
                uint16_t arg0 = 0,
                uint16_t arg1 = 0,
                uint16_t arg2 = 0);
    
    int addInstruction(unsigned char op,
                unsigned char subop,
                uint32_t imm,
                uint16_t reg = 0);
    
    void addTranslator(BlockTranslator* translator);
    
    virtual int size();
    virtual void write(Compiler& compiler);

private:
    
    // DISABLED
    BlockTranslator(const BlockTranslator& orig) {}
};

#endif	/* BLOCKTRANSLATOR_H */

