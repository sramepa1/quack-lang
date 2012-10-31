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
    std::list<std::string*>* parameters;

    NBlock* nblock;
    std::list<NStatement*>* statements;
    NStatement* nstatement;

    NExpression* nexpression;

    std::string* string;
    int token;
}

/* Define our terminal symbols (tokens). This should
   match our tokens.l lex file. We also define the node type
   they represent.
 */
%token <token> K_STATCLASS K_CLASS K_EXTENDS K_FIELD K_INIT K_METHOD
%token <token> T_ASSIGN K_IF K_ELSE K_FOR

%token <token> T_AND T_OR T_NOT

%token <string> T_IDENTIFIER 

%token <token> T_LPAREN T_RPAREN T_COMMA
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
%type <parameters> parameters parameter_list

%type <nblock> block

%type <statements> statement_list
%type <nstatement> statement

%type <nexpression> parameter expression

%type <string> field


/* Operator precedence for mathematical operators */
%left T_PLUS T_MINUS
%left T_MUL T_DIV T_MOD

/* Starting expression of the grammar */
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
  | /* epsilon */
;

dyn_class_entry: 
    field {$$ = new ClassEntry(FIELD, $1);}
  | dyn_init {$$ = new ClassEntry(INIT, $1);}
  | method {$$ = new ClassEntry(METHOD, $1);}
  | /* epsilon */
;

stat_init:
    K_INIT block {$$ = new NMethod(); $$->parameters = NULL; $$->block = $2;}
;

dyn_init:
    K_INIT parameters block {$$ = new NMethod(); $$->parameters = $2; $$->block = $3;}
;

field:
    K_FIELD T_IDENTIFIER T_SEMICOLON {$$ = $2;}
;

method: 
    K_METHOD T_IDENTIFIER parameters block {$$ = new NMethod(); $$->parameters = $3; $$->block = $4;}
;

parameters:
    T_LPAREN parameter_list T_RPAREN {$$ = $2;}
  | T_LPAREN T_RPAREN {$$ = NULL;}
;

parameter_list:
    parameter {$$ = new std::list<std::string*>(); $$->push_back($1);}
  | parameter_list T_COMMA parameter {$$->push_back($3);}
;

parameter:
    expression {$$ = $1;}
;

block:
    T_LBLOCK statement_list T_RBLOCK {$$ = new NBlock(); $$->statements = $2}
  | T_LBLOCK T_RBLOCK {$$ = NULL}
;

statement_list:
    statement T_SEMICOLON {$$ = new std::list<NStatement*>(); $$->push_back($1);}
  | statement_list statement T_SEMICOLON {$$->push_back($2)}
;

statement:
    assignment {$$ = $1;}
  | call {$$ = $1;}
  | if {$$ = $1;}
  | for {$$ = $1;}
  | /* epsilon */
;   

assignment:
    variable T_ASSIGN expression {$$ = }
;

call:
    T_IDENTIFIER parameters

if:
    K_IF T_LPAREN expression T_RPAREN block
  | K_IF T_LPAREN expression T_RPAREN block K_ELSE block
;

for:
   K_FOR T_LPAREN statement T_SEMICOLON expression T_SEMICOLON statement T_RPAREN block
; 

expression:
    T_LPAREN expression T_RPAREN {$$ = $2;}
  | logic_expr
;

logic_expr:
  | T_LPAREN logic_expr T_RPAREN
  | logic_expr T_AND logic_expr
  | logic_expr T_OR logic_expr
  | T_NEG logic_expr
  | compare_expr
;

compare_expr:
    T_LPAREN compare_expr T_RPAREN
  | compare_expr T_EQ compare_expr
  | compare_expr T_NE compare_expr
  | boolean_const
  | string_expr
  | relation_expr
;

string_expr:
    string_expr T_PLUS string_expr
  | string_const
;

relation_expr:
  | T_LPAREN relation_expr T_RPAREN
  | artim_expr T_LT artim_expr
  | artim_expr T_LE artim_expr
  | artim_expr T_GT artim_expr
  | artim_expr T_GE artim_expr
  | artim_expr
;

artim_expr:
    artim_expr T_PLUS artim_expr2
  | artim_expr T_MINUS artim_expr2
  | artim_expr2
;

artim_expr2:
    artim_expr2 T_MUL artim_expr3
  | artim_expr2 T_DIV artim_expr3
  | artim_expr2 T_MOD artim_expr3
  | artim_expr3
;

artim_expr3:
    T_LPAREN artim_expr T_RPAREN
  | artim_const
  | value
;    


artim_const:
    C_INTEGER
  | C_FLOAT
;

boolean_const:
    K_TRUE
  | K_FALSE
;

string_const:
    T_STRING
;

variable:
    indentifier 
    indentifier T_FACCESS indentifier
;

value: 
    temp_var
    variable
;

temp_var:
    indentifier
    indentifier T_MACCESS indentifier
;

indentifier:
    T_IDENTIFIER
;

%%


/*

    assignment T_SEMICOLON
    if
    for
  | T_SEMICOLON

*/