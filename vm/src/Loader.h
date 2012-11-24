#ifndef LOADER_H
#define LOADER_H

#include <vector>
#include <utility>
#include <cstdlib>
extern "C" {
    #include <stdint.h>
}

class Loader
{
public:
    Loader();
    ~Loader();

    void loadNative();
    uint16_t loadClassFile(const char* cfName);

    static bool checkIdentifier(const char* id);

private:

    std::vector<std::pair<void*, size_t> >* mmapedClsFiles;
    uint16_t mainClassType;

    void parseClass(char* start, void* poolPtr, void* clsTablePtr);

};

#endif // LOADER_H
