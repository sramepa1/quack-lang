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

    static bool checkIdentifier(const char* id);

private:

    std::vector<std::pair<void*, size_t> >* mmapedClsFiles;
    Instruction* entryPoint;

    void parseClass(char* start, void* poolPtr, void* clsTablePtr);

};

#endif // LOADER_H
