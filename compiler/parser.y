%{
    #include "../AST.h"
    #include <cstdio>
    #include <cstdlib>

    NProgram* quackProgram; /* the top level root node of the final AST */

    extern int yylex();
    extern int yylineno;
    void yyerror(const char *s) { std::printf("Error: %s\n on line %i\n", s, yylineno);std::exit(1); }
    
%}

/* Enable better error reporting */
%error-verbose

/* Represents the many different ways we can access our data */
%union {
    Node* node;
    NProgram* nprogram;
    NClass* nclass;
    std::list<ClassEntry*>* classEntries;
    ClassEntry* classEntry;
    NMethod* nmethod;
    std::list<std::string*>* methodParameters;
    std::string* string;
    int token;
}

/* Define our terminal symbols (tokens). This should
   match our tokens.l lex file. We also define the node type
   they represent.
 */
%token <token> K_STATCLASS K_CLASS K_EXTENDS K_FIELD K_INIT K_METHOD

%token <string> T_IDENTIFIER 

%token <token> T_LBRACKET T_RBRACKET T_COMMA
%token <token> T_LBLOCK T_RBLOCK T_SEMICOLON


/* Define the type of node our nonterminal symbols represent.
   The types refer to the %union declaration above. Ex: when
   we call an ident (defined by union type ident) we are really
   calling an (NIdentifier*). It makes the compiler happy.
 */

%type <nprogram> program classes
%type <nclass> class class_inheritance
%type <classEntries> stat_class_entries dyn_class_entries
%type <classEntry> stat_class_entry dyn_class_entry 
%type <nmethod> method stat_init dyn_init
%type <methodParameters> parameters parameter_list

%type <string> field parameter


/* Operator precedence for mathematical operators 
%left TPLUS TMINUS
%left TMUL TDIV
*/

%start program

%%

program : 
    classes {quackProgram = $1;}
;

classes: 
    class {$$ = new NProgram(); $$->addClass($1->name, $1);}
  | classes class {$$->addClass($2->name, $2);}
;

class:
    K_STATCLASS T_IDENTIFIER T_LBLOCK stat_class_entries T_RBLOCK {$$ = new NStatClass(); $$->name = $2; $$->entries = $4;}
  | K_CLASS class_inheritance T_LBLOCK dyn_class_entries T_RBLOCK {$$ = $2; $$->entries = $4;}
;

class_inheritance:
    T_IDENTIFIER {$$ = new NDynClass(); $$->name = $1;}
  | T_IDENTIFIER K_EXTENDS T_IDENTIFIER {NDynClass* tmp = new NDynClass(); $$ = tmp; tmp->name = $1; tmp->superClass = $3;}
;

stat_class_entries:
    stat_class_entry {$$ = new std::list<ClassEntry*>(); $$->push_back($1);}
  | stat_class_entries stat_class_entry {$$->push_back($2);}
;

dyn_class_entries:
    dyn_class_entry {$$ = new std::list<ClassEntry*>(); $$->push_back($1);}
  | dyn_class_entries dyn_class_entry {$$->push_back($2);}
;

stat_class_entry: 
    field {$$ = new ClassEntry(FIELD, $1);}
  | stat_init {$$ = new ClassEntry(INIT, $1);}
  | method {$$ = new ClassEntry(METHOD, $1);}
;

dyn_class_entry: 
    field {$$ = new ClassEntry(FIELD, $1);}
  | dyn_init {$$ = new ClassEntry(INIT, $1);}
  | method {$$ = new ClassEntry(METHOD, $1);}
;

stat_init:
    K_INIT block {$$ = NULL;}
;

dyn_init:
    K_INIT parameters block {$$ = NULL;}
;

field:
    K_FIELD T_IDENTIFIER T_SEMICOLON {$$ = $2;}
;

method: 
    K_METHOD T_IDENTIFIER parameters block {$$ = new NMethod(); $$->parameters = $3;}
;

parameters:
    T_LBRACKET parameter_list T_RBRACKET {$$ = $2;}
  | T_LBRACKET T_RBRACKET {$$ = NULL;}
;

parameter_list:
    parameter {$$ = new std::list<std::string*>(); $$->push_back($1);}
  | parameter_list T_COMMA parameter {$$->push_back($3);}
;

parameter:
    T_IDENTIFIER {$$ = $1;}
;

block:
    T_LBLOCK statement_list T_RBLOCK
;

statement_list:
    statement 
  | statement_list statement
;

statement:
    T_SEMICOLON
;   

%%
