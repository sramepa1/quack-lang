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
        *flags = CLS_DESTRUCTIBLE;

        createMethod((QuaSignature*)"\1init", __extension__ (void*)&FileNative::initNativeImpl);
        createMethod((QuaSignature*)"\0readLine", __extension__ (void*)&FileNative::readLineNativeImpl);
        createMethod((QuaSignature*)"\1writeLine", __extension__ (void*)&FileNative::writeLineNativeImpl);
        createMethod((QuaSignature*)"\0eof", __extension__ (void*)&FileNative::eofNativeImpl);
        createMethod((QuaSignature*)"\0close", __extension__ (void*)&FileNative::closeNativeImpl);

    }

    static QuaValue initNativeImpl();       // native init(filename);
    static QuaValue readLineNativeImpl();   // native fun readLine();
    static QuaValue writeLineNativeImpl();  // native fun writeLine(string);
    static QuaValue eofNativeImpl();        // native fun eof();
    static QuaValue closeNativeImpl();      // native fun close();
};

class OutFileNative : protected NativeLoader
{
public:
    OutFileNative() : NativeLoader("OutFile", 0) {

        *parent = typeArray[linkedTypes->at("File")];
        *flags = CLS_DESTRUCTIBLE;

        createMethod((QuaSignature*)"\1init", __extension__ (void*)&OutFileNative::initNativeImpl);
        createMethod((QuaSignature*)"\0readLine", __extension__ (void*)&OutFileNative::readLineNativeImpl);

    }

    static QuaValue initNativeImpl();
    static QuaValue readLineNativeImpl();   // throws an exception :)
};

class InFileNative : protected NativeLoader
{
public:
    InFileNative() : NativeLoader("InFile", 0) {

        *parent = typeArray[linkedTypes->at("File")];
        *flags = CLS_DESTRUCTIBLE;

        createMethod((QuaSignature*)"\1init", __extension__ (void*)&InFileNative::initNativeImpl);
        createMethod((QuaSignature*)"\1writeLine", __extension__ (void*)&InFileNative::writeLineNativeImpl);

    }

    static QuaValue initNativeImpl();
    static QuaValue writeLineNativeImpl();   // throws an exception :)
};

#endif // FILENATIVE_H
