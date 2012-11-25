/* 
 * File:   Compiler.h
 * Author: rusty
 *
 * Created on 17. listopad 2012, 15:41
 */

#ifndef COMPILER_H
#define	COMPILER_H

#include "ConstantPool.h"
#include "ClassTable.h"

#include <fstream>
#include <list>
#include <vector>

// types with wknown size
extern "C" {
   #include <stdint.h>
}



class Compiler {
public:
    Compiler();
    virtual ~Compiler();
    
    const char* outputFileName;
    
  //  void compile();
    
    
//private:
    
    std::ofstream ofs;
    int offset;

 //   void writeConstantPool();
  //  void writeClassTable();
    
    void write(const char* bytes, int lenght);
    void write(const char* c_str);
    void write(const char byte);
    void writeAlign8();

    // DISABLED
    Compiler(const Compiler& orig);
};

#define BYTE_ALLIGN 0

#endif	/* COMPILER_H */

