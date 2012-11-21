#include "Loader.h"

#include <stdexcept>
#include <cctype>

#ifdef DEBUG
#include <iostream>
#endif

#include "globals.h"

extern "C" {
    #include <unistd.h>
    #include <sys/mman.h>
    #include <sys/stat.h>
    #include <fcntl.h>
    #include <stdint.h>
}

#include "NativeLoader.h"
#include "natives/SystemNative.h"

using namespace std;

Loader::Loader() : mmapedClsFiles(new vector<pair<void*, size_t> >()), entryPoint(NULL) {
#ifdef DEBUG
    cout << "Start loading native classes!" << endl;
#endif

    // TODO: load natives
    DataBlobNative();
    SystemNative();

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
        throw runtime_error(string("Class file ") + cfName + " cannot be opened"
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
    char magicNum[] = {0xD, 'U', 0xC, 'K'};

    // this is needed for class files smaller then 1024 bytes
    uint64_t* magicNumBound;
    if(fileStatus.st_size < 1024) {
        magicNumBound = (uint64_t*)((char*)classFileBase + fileStatus.st_size);
    } else {
        magicNumBound = (uint64_t*)((char*)classFileBase + 1024);
    }

    // find magic number
    for(uint64_t* p = (uint64_t*)classFileBase; p < magicNumBound; ++p) {
        if(*(uint32_t*)magicNum == *(uint32_t*)p) {
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
    uint16_t clsFileVersion = *(uint16_t*)curPos;
    curPos += 2;
    uint16_t minVmVersion = *(uint16_t*)curPos;
    curPos += 2;
    
    // TODO: check VM version
    
    char* classTablePos = (char*)classFileBase + *(uint32_t*)curPos;
	curPos += 4;
    char* constantPoolPos = (char*)classFileBase + *(uint32_t*)curPos;
    
    // start parsing classes
    curPos = classTablePos;
    uint32_t classTableSize = *(uint32_t*)curPos;
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
}

Instruction* Loader::getEntryPoint() {
	if(entryPoint) {
		return entryPoint;
	} else {
        throw runtime_error("No entry point found!");
	}
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
	
    typeArray[linkedTypes->size()] =
            new QuaClass(poolPtr, (char*)mmapedClsFiles->back().first + classOffset, className, clsTablePtr);

    // find entry point
    // TODO: check if class main is statclass
    if(className == "Main") {
        QuaSignature* mainSignature = (QuaSignature*)"\1main";

        if(!(typeArray[linkedTypes->size()]->methods.count(mainSignature))) {
            throw runtime_error("The main(args) method was not found within the class Main!");
        }
        entryPoint = (Instruction*) typeArray[linkedTypes->size()]->methods.at(mainSignature)->code;
    }

    linkedTypes->insert(make_pair(className, linkedTypes->size()));

#ifdef DEBUG
    cout << "Class " << className << " constructed!" << endl;
#endif

}

QuaClass::QuaClass(void* constantPool, void* classDef, const string& className, void* clsTabPtr)
    : relevantCP(constantPool) {

    char* curPos = (char*)classDef;

    // ancestor
    // the addend '1' is here because is needed to jump over the class table header
    uint16_t ancestorIndex = *(uint16_t*)((uint64_t*)clsTabPtr + 1 + *(uint16_t*)curPos);
    string ancestorName(Loader::getConstantPoolEntry(constantPool, ancestorIndex));

    if(className == ancestorName) {
        parent = NULL;
    } else {
        if(!linkedTypes->count(ancestorName)) {
            throw runtime_error("The ancestor of class " + className + " was not found!");
        }
        parent = typeArray[linkedTypes->at(ancestorName)];
    }

    // flags
    curPos += 2;
    flags = *(uint16_t*)curPos;
    // TODO: check static class inheritance

    // fields
    curPos += 2;
    myFieldCount = *(uint16_t*)curPos;

#ifdef DEBUG
    cout << "Class " << className << " - field count: " << myFieldCount << endl;
#endif

    curPos += 2;
    for(uint16_t i = 0; i < myFieldCount; ++i) {
        const char* fieldEntry = Loader::getConstantPoolEntry(constantPool, ((uint16_t*)curPos)[i]);
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
        fieldIndices.insert(make_pair(string(fieldEntry), ancestorFieldCount + fieldIndices.size()));
    }

    // methods
    curPos += myFieldCount * sizeof(uint16_t);
    uint16_t methodCount = *(uint16_t*)curPos;

#ifdef DEBUG
    cout << "Class " << className << " - method count: " << methodCount << endl;
#endif

    curPos += 2;
    for(uint16_t i = 0; i < methodCount; ++i) {
        uint16_t signatureIndex = ((uint16_t*)curPos)[2 * i];
        uint16_t codeIndex = ((uint16_t*)curPos)[2 * i + 1];
        QuaSignature* signature = (QuaSignature*)Loader::getConstantPoolEntry(constantPool, signatureIndex);

        if(!Loader::checkIdentifier(signature->name)) {
            throw runtime_error("Method name is invalid!");
        }

#ifdef DEBUG
        cout << "Constructing method " << signature->name << " with " << (int)signature->argCnt << " arg(s)"<< endl;
#endif

        QuaMethod* method = new QuaMethod();
        method->action = QuaMethod::INTERPRET;
        method->code = (void*)Loader::getConstantPoolEntry(constantPool, codeIndex);

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
