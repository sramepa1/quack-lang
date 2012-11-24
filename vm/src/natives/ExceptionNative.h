#ifndef EXCEPTIONNATIVE_H
#define EXCEPTIONNATIVE_H

#include "NativeLoader.h"

class ExceptionNative : protected NativeLoader {
public:
    ExceptionNative() : NativeLoader("Exception", 1) {
        *parent = NULL;
        *flags = 0;
        fields->insert(std::make_pair("what", (uint16_t)0));     // WAT!! :-O
    }
};

class NotFoundExceptionNative : protected NativeLoader {
public:
    NotFoundExceptionNative() : NativeLoader("NotFoundException", 0) {
        *parent = typeArray[linkedTypes->at("Exception")];
        *flags = 0;
    }
};

class IOExceptionNative : protected NativeLoader {
public:
    IOExceptionNative() : NativeLoader("IOException", 0) {
        *parent = typeArray[linkedTypes->at("Exception")];
        *flags = 0;
    }
};

#endif // EXCEPTIONNATIVE_H
