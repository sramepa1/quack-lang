#ifndef LOADER_H
#define LOADER_H

#include "Instruction.h"

#include <vector>
#include <utility>
#include <cstdlib>

class Loader
{
public:
    Loader();
    ~Loader();

    void loadNative();
    void loadClassFile(const char* cfName);

    Instruction* getEntryPoint();
    
    void getConstantPoolEntry(void* poolPtr, uint32_t index, void*& entryPtr, int& entryLength);

private:

    // TODO private helper parsing methods
    std::vector<std::pair<void*, size_t> >* mmapedClsFiles;
    Instruction* entryPoint;

	void parseClass(char* start, void* poolPtr);

};

#endif // LOADER_H
