#include "Loader.h"

#include <stdexcept>

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

using namespace std;

Loader::Loader() : mmapedClsFiles(new vector<pair<void*, size_t> >()), entryPoint(NULL) {
    // TODO: load natives
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
                            "(check if the file exists and you have permission to read it)");
    }

#ifdef DEBUG
    cout << "Class file: " << cfName << " opened!" << endl;
#endif
    struct stat fileStatus;
    if(fstat(clsFileFD, &fileStatus) == -1) {
        throw runtime_error("Cannot obtain information about class file");
    }

    void* classFileBase = mmap(NULL, fileStatus.st_size, PROT_READ | PROT_WRITE, MAP_PRIVATE, clsFileFD, 0);
    checkMmap(classFileBase, "Cannot mmap class file");

    mmapedClsFiles->push_back(make_pair(classFileBase, fileStatus.st_size));
    if(close(clsFileFD) == -1) {
        throw runtime_error("Cannot close class file after mmaping");
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
    for(uint64_t* i = (uint64_t*)classFileBase; i < magicNumBound; ++i) {
        if(*(uint32_t*)magicNum == *(uint32_t*)i) {
            parsingStartPoint = (char*)((uint32_t*)i + 1);
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
        parseClass((char*)p, constantPoolPos);
    }
    
}

Instruction* Loader::getEntryPoint() {
	if(entryPoint) {
		return entryPoint;
	} else {
        throw runtime_error("No entry point found!");
	}
}

void Loader::getConstantPoolEntry(void* poolPtr, uint32_t index, void*& entryPtr, int& entryLength) {
    // TODO: implement it :)
}

void Loader::parseClass(char* start, void* poolPtr) {
	uint16_t nameIndex = *(uint16_t*)start;
	start += 2;
	uint16_t classSize = *(uint16_t*)start;
	start += 2;
	uint32_t classOffset = *(uint32_t*)start;
	
	if(classSize == 0 && classOffset == 0) {
		// class is defined on another place
		return;
	}
	
    void* nameStart = NULL;
    int nameLength = 0;
    getConstantPoolEntry(poolPtr, nameIndex, nameStart, nameLength);
    string className((char*)nameStart, nameLength);

    if(linkedTypes->count(className)) {
        throw runtime_error("Multiple definition of class " + className);
    }
	
    new(typeArray[linkedTypes->size()]) QuaClass((char*)mmapedClsFiles->back().first + classOffset, poolPtr);

    // find entry point
    if(className == "Main") {
        QuaSignature* entryPointSig = (QuaSignature*)"1main";
        QuaMethod* mainMethod = typeArray[linkedTypes->size()]->lookupMethod(entryPointSig);

        if(!mainMethod) {
            throw runtime_error("The main(args) method was not found within the class Main!");
        }
        entryPoint = (Instruction*) mainMethod->code;
    }

    linkedTypes->insert(make_pair(className, linkedTypes->size()));

}
