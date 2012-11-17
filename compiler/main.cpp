
#include "AST.h"

using namespace std;

extern int yyparse();
extern NProgram* quackProgram;

int main(int argc, char** argv) {
    
    yyparse();
    quackProgram->generateCode();

    return 0;
}

