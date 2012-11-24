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
    void loadClassFile(const char* cfName);
    uint16_t getMainType();

    static bool checkIdentifier(const char* id);

private:

    std::vector<std::pair<void*, size_t> >* mmapedClsFiles;
    uint16_t mainClassType;

    void parseClass(char* start, void* poolPtr, void* clsTablePtr);

};

#endif // LOADER_H
