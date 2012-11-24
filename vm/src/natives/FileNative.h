#ifndef FILENATIVE_H
#define FILENATIVE_H

#include "../NativeLoader.h"

// Flags for File instances
#define FILE_FLAG_CLOSED 1
#define FILE_FLAG_UNCLOSEABLE 2

class FileNative : protected NativeLoader
{
public:

    // 3 hidden fields - first is for flags, second and third are high and low part of std::fstream pointer
    FileNative() : NativeLoader("File", 3) {

        *parent = NULL;
        *flags = 0;

        createMethod((QuaSignature*)"\1init", (void*)&FileNative::initNativeImpl);
        createMethod((QuaSignature*)"\0readLine", (void*)&FileNative::readLineNativeImpl);
        createMethod((QuaSignature*)"\1writeLine", (void*)&FileNative::writeLineNativeImpl);
        createMethod((QuaSignature*)"\0eof", (void*)&FileNative::eofNativeImpl);
        createMethod((QuaSignature*)"\0close", (void*)&FileNative::closeNativeImpl);

    }

    static void initNativeImpl();       // native init(filename);
    static void readLineNativeImpl();   // native fun readLine();
    static void writeLineNativeImpl();  // native fun writeLine(string);
    static void eofNativeImpl();        // native fun eof();
    static void closeNativeImpl();      // native fun close();
};

class OutFileNative : protected NativeLoader
{
public:
    OutFileNative() : NativeLoader("OutFile", 0) {

        *parent = typeArray[linkedTypes->at("File")];
        *flags = 0;

        createMethod((QuaSignature*)"\1init", (void*)&OutFileNative::initNativeImpl);
        createMethod((QuaSignature*)"\0readLine", (void*)&OutFileNative::readLineNativeImpl);

    }

    static void initNativeImpl();
    static void readLineNativeImpl();   // throws an exception :)
};

class InFileNative : protected NativeLoader
{
public:
    InFileNative() : NativeLoader("InFile", 0) {

        *parent = typeArray[linkedTypes->at("File")];
        *flags = 0;

        createMethod((QuaSignature*)"\1init", (void*)&InFileNative::initNativeImpl);
        createMethod((QuaSignature*)"\1writeLine", (void*)&InFileNative::writeLineNativeImpl);

    }

    static void initNativeImpl();
    static void writeLineNativeImpl();   // throws an exception :)
};

#endif // FILENATIVE_H
