/* 
 * File:   Compiler.h
 * Author: rusty
 *
 * Created on 17. listopad 2012, 15:41
 */

#ifndef COMPILER_H
#define	COMPILER_H

#include <fstream>
#include <list>
#include <vector>

// types with wknown size
extern "C" {
   #include <stdint.h>
}

#define ALLIGN_BYTE_VALUE 0

class Compiler {
public:
    Compiler();
    virtual ~Compiler();
    
    const char* outputFileName;
    
    void compile();
    
    
//private:
    
    std::ofstream ofs;
    int offset;

 //   void writeConstantPool();
  //  void writeClassTable();
    
    void write(const char* bytes, int lenght);
    void write(const char* c_str);
    void write(const char byte);
    void writeAlign8();

    static uint32_t sizeToAlign8(uint32_t size);
    
    // DISABLED
    Compiler(const Compiler& orig);
};


class IWritable {
public:
    virtual ~IWritable() {}
    
    virtual int size() = 0;
    virtual void write(Compiler& compiler) = 0;
};





#endif	/* COMPILER_H */

