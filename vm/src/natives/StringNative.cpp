#include "StringNative.h"

#include <stdexcept>

using namespace std;

QuaValue stringDeserializer(const char* data) {
    QuaValue strRef = heap->allocateNew(linkedTypes->at("String"), 2);

}


QuaValue StringNative::init0NativeImpl() {
    throw runtime_error("Native method not yet implemented.");
}


QuaValue StringNative::init1NativeImpl() {
    throw runtime_error("Native method not yet implemented.");
}


QuaValue StringNative::_operatorPlusNativeImpl() {
    throw runtime_error("Native method not yet implemented.");
}


QuaValue StringNative::_operatorIndexNativeImpl() {
    throw runtime_error("Native method not yet implemented.");
}


QuaValue StringNative::_stringValueNativeImpl() {
    throw runtime_error("Native method not yet implemented.");
}


QuaValue StringNative::explodeNativeImpl() {
    throw runtime_error("Native method not yet implemented.");
}
