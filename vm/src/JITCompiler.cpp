#include "JITCompiler.h"
#include "bytecode.h"

#ifdef DEBUG
#include <iostream>
#include <cerrno>
extern "C" {
	#include <unistd.h>
	void* jumpto;
}
#endif

#include <cstring>

extern "C" {
	#include <sys/mman.h>
}

using namespace std;

void* JITCompiler::callLabel;
void* JITCompiler::returnLabel;
void* JITCompiler::throwLabel;
void* JITCompiler::whatLabel;

// Constructor defined in Interpreter.cpp


bool JITCompiler::compile(QuaMethod* method) {

	if(!enabled) {
		return false;
	}

	#ifdef DEBUG
		if(method->action != QuaMethod::COMPILE) {
			cerr << "Warning: Attempted to compile a method with action=" << method->action << " !" << endl;
		}
	#endif

	vector<unsigned char> buffer(4096);
	void* memblob;

	try {
		list<Instruction*> insns = buildObjects(method); //signature may change
		allocateRegisters(insns);
		generate(insns, buffer);

		size_t blobsize = (buffer.size() / getpagesize() + 1) * getpagesize(); // better safe than sorry
		memblob = mmap(NULL, blobsize, PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
		if(memblob == MAP_FAILED) {
			#ifdef DEBUG
				cerr << "Warning: JIT mmap failed with error: " << strerror(errno) << endl;
			#endif
			throw GiveUpException();
		}
		#ifdef DEBUG
			// preventive INT3s
			memset(memblob, 0xCC, blobsize);
		#endif
		memcpy(memblob, buffer.data(), buffer.size());
		if(mprotect(memblob, blobsize, PROT_READ | PROT_EXEC) != 0) {
			#ifdef DEBUG
				cerr << "Warning: JIT mprotect failed with error: " << strerror(errno) << endl;
			#endif
			throw GiveUpException();
		}

	} catch(GiveUpException) {
		return false;
	}

	method->code = memblob;
	return true;
}


list<Instruction*> JITCompiler::buildObjects(QuaMethod* method) {

	list<Instruction*> ilist;

	// TODO: This is only temporary, for testing
	for(Instruction* iptr = (Instruction*)method->code; iptr < (Instruction*)method->code + method->insnCount; iptr++) {
		ilist.push_back(iptr);
	}

	return ilist;
}


void JITCompiler::allocateRegisters(list<Instruction*> insns) {

	//throw GiveUpException();
}


void JITCompiler::generate(list<Instruction*> insns, vector<unsigned char> & buffer) {

	// TODO: This is only temporary, for testing
	int i = 0;
	for(list<Instruction*>::iterator it = insns.begin(); it != insns.end(); ++it) {

		switch((*it)->op) {
			case OP_NOP:
				buffer[i++] = 0x90;
				break;
			case OP_RETNULL:

				buffer[i++] = 0x48;
				buffer[i++] = 0xB8; //mov rax, imm64
				for(int j = 0; j < 8; j++) {
					buffer[i++] = ((char*)&whatLabel)[j];
				}
				buffer[i++] = 0x48;
				buffer[i++] = 0xC7;
				buffer[i++] = 0;	// mov qword [rax], imm32
				for(int j = 0; j < 4; j++) {
					buffer[i++] = 0;
				}

				buffer[i++] = 0x48;
				buffer[i++] = 0xB8; //mov rax, imm64
				for(int j = 0; j < 8; j++) {
					buffer[i++] = ((char*)&returnLabel)[j];
				}
				buffer[i++] = 0xFF;
				buffer[i++] = 0xE0; // jmp rax
				break;
			default: throw GiveUpException();
		}
	}
}

