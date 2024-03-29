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

#include <map>
#include <vector>

class BlockTranslator : public IWritable {
public:
    BlockTranslator(std::map<std::string, uint16_t>* localVariables);
    virtual ~BlockTranslator();
    
    std::vector<Instruction*> instructions;

    // this is the first register which is not used for local variables
    uint16_t initialRegister;

    // register which is available for temporary purpurose (e.g. in expressions)
    uint16_t firstUnusedRegister;

    // the highest register which is used in this method
    uint16_t usedRegisterCount;
    
    std::map<std::string, uint16_t>* localVariables;

    uint16_t getFreeRegister() {
        // TODO: test for register overflow
        if(firstUnusedRegister >= usedRegisterCount) {
            usedRegisterCount = firstUnusedRegister + 1;
        }

        return firstUnusedRegister++;
    }
    
    uint16_t getUsedRegisterCount() {
        return usedRegisterCount;
    }

    void resetRegisters() {
        firstUnusedRegister = initialRegister;
    }
    
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
    
    int instrCount();

private:
    
    // DISABLED
    BlockTranslator(const BlockTranslator& orig) {}
};

#endif	/* BLOCKTRANSLATOR_H */

