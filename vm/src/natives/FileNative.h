#ifndef FILENATIVE_H
#define FILENATIVE_H

#include "NativeLoader.h"

// Flags for File instances
#define FILE_FLAG_CLOSED 1
#define FILE_FLAG_UNCLOSEABLE 2

class FileNative
{
    const char* const name;

public:

    FileNative(NativeLoader* loader) : name("File") {
        loader->registerNativeMethod(name, (QuaSignature*)"\1initN",
                          __extension__ (void*)&FileNative::initNativeImpl);
        loader->registerNativeMethod(name, (QuaSignature*)"\0readLine",
                          __extension__ (void*)&FileNative::readLineNativeImpl);
        loader->registerNativeMethod(name, (QuaSignature*)"\1writeLineN",
                          __extension__ (void*)&FileNative::writeLineNativeImpl);
        loader->registerNativeMethod(name, (QuaSignature*)"\0eof",
                          __extension__ (void*)&FileNative::eofNativeImpl);
        loader->registerNativeMethod(name, (QuaSignature*)"\0finalize",
                          __extension__ (void*)&FileNative::finalizeNativeImpl);
    }

    static QuaValue initNativeImpl();       // native init(filename);
    static QuaValue readLineNativeImpl();   // native fun readLine();
    static QuaValue writeLineNativeImpl();  // native fun writeLine(string);
    static QuaValue eofNativeImpl();        // native fun eof();
    static QuaValue finalizeNativeImpl();   // native fun finalize();
};

class OutFileNative
{
    const char* const name;

public:
    OutFileNative(NativeLoader* loader) : name("OutFile") {
        loader->registerNativeMethod(name, (QuaSignature*)"\1initN",
                          __extension__ (void*)&OutFileNative::initNativeImpl);
    }

    static QuaValue initNativeImpl();
};

class InFileNative
{
    const char* const name;

public:
    InFileNative(NativeLoader* loader) : name("InFile") {
        loader->registerNativeMethod(name, (QuaSignature*)"\1initN",
                          __extension__ (void*)&InFileNative::initNativeImpl);
    }

    static QuaValue initNativeImpl();

};

#endif // FILENATIVE_H
