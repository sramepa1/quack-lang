%{ 
#include "../AST.h"
#include "parser.hpp"
#define SAVE_TOKEN yylval.string = new std::string(yytext, yyleng)
#define TOKEN(t) (yylval.token = t)
extern "C" int yywrap() { return 1; }
%}

%option yylineno

%x C_COMMENT

%%
[ \t\n]                                                 ; /* ignore whitespaces */
\/\/.*\n                                                  ;


"/*"            { BEGIN(C_COMMENT); }
<C_COMMENT>"*/" { BEGIN(INITIAL); }
<C_COMMENT>.    { }


"class"                                 return TOKEN(K_CLASS);
"statclass"                             return TOKEN(K_STATCLASS);
"extends"                               return TOKEN(K_EXTENDS);
"field"                                 return TOKEN(K_FIELD);
"init"                                  return TOKEN(K_INIT);
"fun"                                   return TOKEN(K_METHOD);
"if"                                    return TOKEN(K_IF);
"else"                                  return TOKEN(K_ELSE);
"for"                                   return TOKEN(K_FOR);

";"                                     return TOKEN(T_SEMICOLON);
"("                                     return TOKEN(T_LBRACKET);
")"                                     return TOKEN(T_RBRACKET);
"{"                                     return TOKEN(T_LBLOCK);
"}"                                     return TOKEN(T_RBLOCK);
","                                     return TOKEN(T_COMMA);

[a-zA-Z_][a-zA-Z0-9_]*                  SAVE_TOKEN; return T_IDENTIFIER ;

.					printf("Unknown token!\n"); yyterminate();

%%
