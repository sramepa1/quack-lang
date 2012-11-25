#include "NativeLoader.h"

#include <stdexcept>

using namespace std;

NativeLoader::NativeLoader() {
}

void* NativeLoader::getNativeMethod(string className, QuaSignature* methodSig) {
    throw runtime_error("Not yet implemented.");
}



