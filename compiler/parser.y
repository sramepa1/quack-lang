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
    ClassEntry* nclassEntry;

    NField* nfield;
    NMethod* nmethod;
    std::list<NExpression*>* parameters;

    NBlock* nblock;
    std::list<NStatement*>* statements;

    NStatement* sstatement;
    SAssignment* sassignment;
    NCall* ncall;
    SIf* sif;
    SFor* sfor;

    NExpression* nexpression;

    std::string* string;
    int integer;
    bool boolean;
    float real;
    int token;
}

/* Define our terminal symbols (tokens). This should
   match our tokens.l lex file. We also define the node type
   they represent.
 */
%token <token> K_STATCLASS K_CLASS K_EXTENDS K_FIELD K_INIT K_METHOD K_FLAGS
%token <token> K_THIS K_NULL

%token <token> T_ASSIGN K_IF K_ELSE K_FOR

%token <token> T_AND T_OR T_NOT
%token <token> T_EQ T_NE T_LT T_LE T_GT T_GE
%token <token> T_PLUS T_MINUS T_MUL T_DIV T_MOD

%token <token> T_MACCESS T_FACCESS T_STATIC

%token <string> T_IDENTIFIER T_STRING C_INTEGER C_HEX_INTEGER C_FLOAT

%token <token> T_LPAREN T_RPAREN T_COMMA
%token <token> T_LBLOCK T_RBLOCK T_SEMICOLON

%token <bool> K_TRUE K_FALSE

/* Define the type of node our nonterminal symbols represent.
   The types refer to the %union declaration above. Ex: when
   we call an ident (defined by union type ident) we are really
   calling an (NIdentifier*). It makes the compiler happy.
 */

%type <nprogram> program classes

%type <nclass> class class_inheritance
%type <classEntries> stat_class_entries dyn_class_entries
%type <nclassEntry> stat_class_entry dyn_class_entry 

%type <nfield> field
%type <nmethod> method stat_init dyn_init
%type <parameters> parameters parameter_list

%type <nblock> block

%type <statements> statement_list
%type <sstatement> block_statement standalone_statement
%type <sassignment> assignment
%type <ncall> call
%type <sif> if
%type <sfor> for

%type <nexpression> parameter expression logic_expr compare_expr string_expr relation_expr artim_expr artim_expr2 artim_expr3
%type <nexpression> artim_const boolean_const string_const value variable

/* %type <string> */



/* Operator precedence for mathematical operators */
%left T_NOT
%left T_AND T_OR
%left T_EQ T_NE
%left T_LT T_LE T_GT T_GE
%left T_PLUS T_MINUS
%left P_UMINUS
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
  | K_STATCLASS T_IDENTIFIER T_LBLOCK T_RBLOCK {$$ = new NStatClass(); $$->name = $2; $$->entries = new std::list<ClassEntry*>();}
  | K_CLASS class_inheritance T_LBLOCK T_RBLOCK {$$ = $2; $$->entries = new std::list<ClassEntry*>();}
;

class_inheritance:
    T_IDENTIFIER {$$ = new NDynClass(); $$->name = $1;}
  | T_IDENTIFIER K_EXTENDS T_IDENTIFIER {NDynClass* tmp = new NDynClass(); $$ = tmp; tmp->name = $1; tmp->ancestor = $3;}
  | T_IDENTIFIER K_FLAGS C_HEX_INTEGER {NDynClass* tmp = new NDynClass(); $$ = tmp; tmp->name = $1; tmp->flags = (uint16_t) strtol($3->c_str(), NULL, 16); delete $3;}
  | T_IDENTIFIER K_EXTENDS T_IDENTIFIER K_FLAGS C_HEX_INTEGER {NDynClass* tmp = new NDynClass(); $$ = tmp; tmp->name = $1; tmp->ancestor = $3; tmp->flags = (uint16_t) strtol($5->c_str(), NULL, 16); delete $5;}
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
    field {$$ = $1;}
  | stat_init {$$ = $1;}
  | method {$$ = $1;}
;

dyn_class_entry: 
    field {$$ = $1;}
  | dyn_init {$$ = $1;}
  | method {$$ = $1;}
;

field:
    K_FIELD T_IDENTIFIER T_SEMICOLON {$$ = new NField($2);}
  | K_FIELD T_IDENTIFIER K_FLAGS C_HEX_INTEGER T_SEMICOLON {$$ = new NField($2, (uint16_t) strtol($4->c_str(), NULL, 16)); delete $4;}
;

stat_init:
    K_INIT block {$$ = new NMethod(new std::string("init"), NULL, $2);}
;

dyn_init:
    K_INIT parameters block {$$ = new NMethod(new std::string("init"), $2, $3);}
;

method: 
    K_METHOD T_IDENTIFIER parameters block {$$ = new NMethod($2, $3, $4);}
  | K_METHOD T_IDENTIFIER parameters K_FLAGS C_HEX_INTEGER  block {$$ = new NMethod($2, $3, $6, (uint16_t) strtol($5->c_str(), NULL, 16)); delete $5;}
;

parameters:
    T_LPAREN parameter_list T_RPAREN {$$ = $2;}
  | T_LPAREN T_RPAREN {$$ = NULL;}
;

parameter_list:
    parameter {$$ = new std::list<NExpression*>(); $$->push_back($1);}
  | parameter_list T_COMMA parameter {$$->push_back($3);}
;

parameter:
    expression {$$ = $1;}
;

block:
    T_LBLOCK statement_list T_RBLOCK {$$ = new NBlock(); $$->statements = $2;}
  | T_LBLOCK T_RBLOCK {$$ = NULL;}
;

statement_list:
    block_statement {$$ = new std::list<NStatement*>(); $$->push_back($1);}
  | statement_list block_statement {$$->push_back($2);}
;

block_statement:
    assignment T_SEMICOLON {$$ = $1;}
  | call T_SEMICOLON {$$ = $1;}
  | if {$$ = $1;}
  | for {$$ = $1;}
  | T_SEMICOLON {$$ = NULL;}
;

standalone_statement:
    assignment  {$$ = $1;}
  | call {$$ = $1;}
  | if {$$ = $1;}
  | for {$$ = $1;}
;   

assignment:
    variable T_ASSIGN expression {$$ = new SAssignment(); $$->variable = (EVarible*) $1; $$->expression = $3;}
;

call:
    T_IDENTIFIER parameters {$$ = new NCall(); $$->methodName = $1; $$->parameters = $2;}
;

/*
dynamic_access:
    K_THIS
  | T_IDENTIFIER access
  | T_IDENTIFIER access
;

static_access:
    T_STATIC T_IDENTIFIER T_FACCESS
  | T_STATIC T_IDENTIFIER T_MACCESS
;

access:
    T_FACCESS
  | T_MACCESS
;
*/

if:
    K_IF T_LPAREN expression T_RPAREN block {$$ = new SIf(); $$->condition = $3; $$->thenBlock = $5;}
  | K_IF T_LPAREN expression T_RPAREN block K_ELSE block {$$ = new SIf(); $$->condition = $3; $$->thenBlock = $5; $$->elseBlock = $7;}
;

for:
   K_FOR T_LPAREN standalone_statement T_SEMICOLON expression T_SEMICOLON standalone_statement T_RPAREN block {$$ = new SFor(); $$->init = $3; $$->condition = $5; $$->increment = $7; $$->body = $9;}
; 

expression:
    T_LPAREN expression T_RPAREN {$$ = $2;}
  | logic_expr {$$ = $1;}
;

logic_expr:
    T_LPAREN logic_expr T_RPAREN {$$ = $2;}
  | logic_expr T_AND logic_expr {$$ = new EAnd(); ((EBOp*) $$)->left = $1; ((EBOp*) $$)->right = $3;}
  | logic_expr T_OR logic_expr {$$ = new EOr(); ((EBOp*) $$)->left = $1; ((EBOp*) $$)->right = $3;}
  | T_NOT logic_expr {$$ = new ENot(); ((EUOp*) $$)->expr = $2;}
  | compare_expr {$$ = $1;}
;

compare_expr:
    T_LPAREN compare_expr T_RPAREN {$$ = $2;}
  | compare_expr T_EQ compare_expr {$$ = new EEq(); ((EBOp*) $$)->left = $1; ((EBOp*) $$)->right = $3;}
  | compare_expr T_NE compare_expr {$$ = new ENe(); ((EBOp*) $$)->left = $1; ((EBOp*) $$)->right = $3;}
  | boolean_const {$$ = $1;}
  | string_expr {$$ = $1;}
  | relation_expr {$$ = $1;}
    /* heere add potencial "instanceof" */
;

string_expr:
    string_expr T_PLUS string_expr {$$ = new EAdd(); ((EBOp*) $$)->left = $1; ((EBOp*) $$)->right = $3;}
  | string_const {$$ = $1;}
;

relation_expr:
    T_LPAREN relation_expr T_RPAREN {$$ = $2;}
  | artim_expr T_LT artim_expr {$$ = new ELt(); ((EBOp*) $$)->left = $1; ((EBOp*) $$)->right = $3;}
  | artim_expr T_LE artim_expr {$$ = new ELe(); ((EBOp*) $$)->left = $1; ((EBOp*) $$)->right = $3;}
  | artim_expr T_GT artim_expr {$$ = new EGt(); ((EBOp*) $$)->left = $1; ((EBOp*) $$)->right = $3;}
  | artim_expr T_GE artim_expr {$$ = new EGe(); ((EBOp*) $$)->left = $1; ((EBOp*) $$)->right = $3;}
  | artim_expr {$$ = $1;}
;

artim_expr:
    artim_expr T_PLUS artim_expr2 {$$ = new EAdd(); ((EBOp*) $$)->left = $1; ((EBOp*) $$)->right = $3;}
  | artim_expr T_MINUS artim_expr2 {$$ = new ESub(); ((EBOp*) $$)->left = $1; ((EBOp*) $$)->right = $3;}
/*| T_MINUS artim_expr %prec P_UMINUS {$$ = new ESub();} */
  | artim_expr2 {$$ = $1;}
;

artim_expr2:
    artim_expr2 T_MUL artim_expr3 {$$ = new EMul(); ((EBOp*) $$)->left = $1; ((EBOp*) $$)->right = $3;}
  | artim_expr2 T_DIV artim_expr3 {$$ = new EDiv(); ((EBOp*) $$)->left = $1; ((EBOp*) $$)->right = $3;}
  | artim_expr2 T_MOD artim_expr3 {$$ = new EMod(); ((EBOp*) $$)->left = $1; ((EBOp*) $$)->right = $3;}
  | artim_expr3 {$$ = $1;}
;

artim_expr3:
    T_LPAREN artim_expr T_RPAREN {$$ = $2;}
  | artim_const {$$ = $1;}
  | value {$$ = $1;}
;    

artim_const:
    C_INTEGER {$$ = new CInt(); ((CInt*) $$)->value = atol($1->c_str()); delete $1;}
  | C_FLOAT {$$ = new CFloat(); ((CFloat*) $$)->value = atof($1->c_str()); delete $1;}
;

boolean_const:
    K_TRUE {$$ = new CBool(); ((CBool*) $$)->value = true;}
  | K_FALSE {$$ = new CBool(); ((CBool*) $$)->value = false;}
;

string_const:
    T_STRING {$$ = new CString(); ((CString*) $$)->value = $1;}
;

value: 
    call {$$ = $1;}
  | variable {$$ = $1;}
;

variable:
    T_IDENTIFIER {$$ = new EVarible(); ((EVarible*) $$)->variableName = $1;}
  | T_IDENTIFIER T_FACCESS T_IDENTIFIER {$$ = new EVarible(); ((EVarible*) $$)->className = $1; ((EVarible*) $$)->variableName = $3;}
/*  | T_STATIC T_IDENTIFIER T_FACCESS T_IDENTIFIER {$$ = new EVarible(); ((EVarible*) $$)->className = $2; ((EVarible*) $$)->variableName = $4;} */
;


%%


/*

    assignment T_SEMICOLON
    if
    for
  | T_SEMICOLON

*/