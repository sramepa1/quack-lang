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
    
    static const char* getConstantPoolEntry(void* poolPtr, uint16_t index) {
        uint32_t* offsetsTable = (uint32_t*)((char*)poolPtr + 8);
        return (char*)poolPtr + offsetsTable[index];
    }
    static bool checkIdentifier(const char* id);

private:

    std::vector<std::pair<void*, size_t> >* mmapedClsFiles;
    Instruction* entryPoint;

	void parseClass(char* start, void* poolPtr);

};

#endif // LOADER_H
