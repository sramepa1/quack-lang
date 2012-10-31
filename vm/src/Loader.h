#ifndef LOADER_H
#define LOADER_H

#include "Instruction.h"

class Loader
{
public:
    Loader();

    void loadNative();
    void loadClassFile(const char* cfName);

    Instruction* getEntryPoint();

    // TODO private helper parsing methods
};

#endif // LOADER_H
