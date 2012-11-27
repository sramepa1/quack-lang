#include "Loader.h"

#include <stdexcept>
#include <cctype>

#ifdef DEBUG
#include <iostream>
#endif

#include "classfile.h"

#include "globals.h"
#include "helpers.h"

extern "C" {
    #include <unistd.h>
    #include <sys/mman.h>
    #include <sys/stat.h>
    #include <fcntl.h>
}

#include "NativeLoader.h"

using namespace std;

Loader::Loader() : mmapedClsFiles(new vector<pair<void*, size_t> >()) {
#ifdef DEBUG
    cout << "Start loading native classes!" << endl;
#endif

    // Loading the core classes of Quack runtime
    // This will be replaced by resource embedded into VM's binary
    loadClassFile("./rt.qc");

#ifdef DEBUG
    cout << "Native classes loaded!" << endl;
#endif
}

Loader::~Loader() {
    for(vector<pair<void*, size_t> >::iterator it = mmapedClsFiles->begin(); it != mmapedClsFiles->end(); ++it) {
        munmap(it->first, it->second);
    }
    delete mmapedClsFiles;
}

void Loader::loadClassFile(const char* cfName) {

    int clsFileFD = open(cfName, O_RDONLY);
    if(clsFileFD == -1) {
        throw runtime_error(string("Class file ") + cfName + " cannot be opened "
                            "(check if the file exists and you have permission to read it)!");
    }

#ifdef DEBUG
    cout << "Class file: " << cfName << " opened!" << endl;
#endif
    struct stat fileStatus;
    if(fstat(clsFileFD, &fileStatus) == -1) {
        throw runtime_error("Cannot obtain information about class file!");
    }

    void* classFileBase = mmap(NULL, fileStatus.st_size, PROT_READ | PROT_WRITE, MAP_PRIVATE, clsFileFD, 0);
    checkMmap(classFileBase, "Cannot mmap class file!");

    mmapedClsFiles->push_back(make_pair(classFileBase, fileStatus.st_size));
    if(close(clsFileFD) == -1) {
        throw runtime_error("Cannot close class file after mmaping!");
    }

#ifdef DEBUG
    cout << "Class file: " << cfName << " successfuly loaded into memory!" << endl;
#endif

    char* parsingStartPoint = NULL;

    // this is needed for class files smaller then 1024 bytes
    uint64_t* magicNumBound;
    if(fileStatus.st_size < 1024) {
        magicNumBound = (uint64_t*)((char*)classFileBase + fileStatus.st_size);
    } else {
        magicNumBound = (uint64_t*)((char*)classFileBase + 1024);
    }

    // find magic number
    for(uint64_t* p = (uint64_t*)classFileBase; p < magicNumBound; ++p) {
        if(MAGIC_NUM == *(uint32_t*)p) {
            parsingStartPoint = (char*)((uint32_t*)p + 1);
            break;
        }
    }

    if(!parsingStartPoint) {
        throw runtime_error("Malformed class file!\nCannot find magic number within the first 1024 bytes.");
    }

#ifdef DEBUG
    cout << "Magic number found!" << endl;
#endif

    // helper pointer for better orientation in currently parsed file
    char* curPos = (char*)parsingStartPoint;

    // read classfile version and minimun required VM version
    //uint16_t clsFileVersion = *(uint16_t*)curPos;
    curPos += 2;
    //uint16_t minVmVersion = *(uint16_t*)curPos;
    curPos += 2;

    // TODO: check VM version

    char* classTablePos = (char*)classFileBase + *(uint32_t*)curPos;
    curPos += 4;
    char* constantPoolPos = (char*)classFileBase + *(uint32_t*)curPos;

    // start parsing classes
    curPos = classTablePos;
    //uint32_t classTableSize = *(uint32_t*)curPos;
    curPos += 4;
    uint16_t numberOfClasses = *(uint16_t*)curPos;
    curPos += 4;
    uint64_t* classTableEnd = (uint64_t*)curPos + numberOfClasses;

#ifdef DEBUG
    cout << "Start parsing classes" << endl;
#endif

    for(uint64_t* p = (uint64_t*) curPos; p < classTableEnd; ++p) {
        parseClass((char*)p, constantPoolPos, classTablePos);
    }

#ifdef DEBUG
    cout << "Loading class file " << cfName << " finished." << endl;
#endif
}


void Loader::parseClass(char* start, void* poolPtr, void* clsTablePtr) {
    uint16_t nameIndex = *(uint16_t*)start;
    start += 2;
    uint16_t classSize = *(uint16_t*)start;
    start += 2;
    uint32_t classOffset = *(uint32_t*)start;

    if(classSize == 0 && classOffset == 0) {
        // class is defined on another place
        return;
    }

    const char* entry = getConstantPoolEntry(poolPtr, nameIndex);
    if(!checkIdentifier(entry)) {
        throw runtime_error("Class name is invalid!");
    }

    string className(entry);
    if(linkedTypes->count(className)) {
        throw runtime_error("Multiple definition of class " + className + "!");
    }

#ifdef DEBUG
    cout << "Constructing class " << className << endl;
#endif

    typeArray[linkedTypes->size()] = new QuaClass(poolPtr, (char*)clsTablePtr + classOffset, className, clsTablePtr);
    linkedTypes->insert(make_pair(className, linkedTypes->size()));

#ifdef DEBUG
    cout << "Class " << className << " constructed!" << endl;
#endif

}

QuaClass::QuaClass(void* constantPool, void* classDef, const string& className, void* clsTabPtr)
    : className(className), relevantCP(constantPool), relevantCT(clsTabPtr)  {

    char* curPos = (char*)classDef;

    // parent
    uint16_t parentNameIndex = *((uint16_t *)getClassTableEntry(clsTabPtr, *(uint16_t*)curPos));
    string parentName(getConstantPoolEntry(constantPool, parentNameIndex));

    if(className == parentName) {
        parent = NULL;
    } else {
        if(!linkedTypes->count(parentName)) {
            throw runtime_error("The parent of class " + className + " was not found!");
        }
        parent = typeArray[linkedTypes->at(parentName)];
    }

    // flags
    curPos += 2;
    flags = *(uint16_t*)curPos;
    // TODO: check static class inheritance

    // deserializer
    deserializer = nativeLoader->getClassDeserializer(className);

    // fields
    curPos += 2;
    myFieldCount = *(uint16_t*)curPos;

#ifdef DEBUG
    cout << "Class " << className << " - field count: " << myFieldCount << endl;
#endif

    curPos += 2;
    for(uint16_t i = 0; i < myFieldCount; ++i) {
        uint16_t fieldFlags = ((uint16_t*)curPos)[2*i];
        if(fieldFlags & FIELD_FLAG_HIDDEN) {
            continue;
        }
        const char* fieldEntry = getConstantPoolEntry(constantPool, ((uint16_t*)curPos)[2*i+1]);
        if(!Loader::checkIdentifier(fieldEntry)) {
            throw runtime_error("Field name is invalid!");
        }

#ifdef DEBUG
            cout << "Constructing field " << fieldEntry << endl;
#endif

        uint16_t ancestorFieldCount = 0;
        if(parent) {
            ancestorFieldCount = parent->getFieldCount();
        }

        // TODO: check index overflow!!!
        fieldIndices.insert(make_pair(fieldEntry, ancestorFieldCount + fieldIndices.size()));
    }

    // methods
    curPos += myFieldCount * 2 * sizeof(uint16_t);
    uint16_t methodCount = *(uint16_t*)curPos;

#ifdef DEBUG
    cout << "Class " << className << " - method count: " << methodCount << endl;
#endif

    curPos += 2;
    for(uint16_t i = 0; i < methodCount; ++i) {
        uint16_t methodFlags = ((uint16_t*)curPos)[4 * i];
        uint16_t signatureIndex = ((uint16_t*)curPos)[4 * i + 1];
        uint16_t codeIndex = ((uint16_t*)curPos)[4 * i + 2];
        uint16_t insnCount = ((uint16_t*)curPos)[4 * i + 3];
        QuaSignature* signature = (QuaSignature*)getConstantPoolEntry(constantPool, signatureIndex);

        if(!Loader::checkIdentifier(signature->name)) {
            throw runtime_error(string("Method name \"") + signature->name + "\" is invalid!");
        }

#ifdef DEBUG
        cout << "Constructing method " << signature->name << " with " << (int)signature->argCnt << " arg(s) ";
#endif

        QuaMethod* method = new QuaMethod();
        if(methodFlags & METHOD_FLAG_NATIVE) {
            method->action = QuaMethod::C_CALL;
            method->code = nativeLoader->getNativeMethod(className, signature);
            method->insnCount = 0;

#ifdef DEBUG
            cout << " - native" << endl;
#endif
        } else {
            method->action = QuaMethod::INTERPRET;
            method->code = (void*)getConstantPoolEntry(constantPool, codeIndex);
            method->insnCount = insnCount;

#ifdef DEBUG
            cout << " - " << insnCount << " instruction(s)" << endl;
#endif
        }

        methods.insert(make_pair(signature, method));
    }
}

bool Loader::checkIdentifier(const char* id) {

    int index = 0;
    unsigned char current = id[index];
    if(!(isalpha(current) || current == '_')) {
        return false;
    }

    current = id[++index];
    while(current != '\0') {
        if(!(isalnum(current) || current == '_')) {
            return false;
        }
        current = id[++index];
    }

    return true;
}
