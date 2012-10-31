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
\/\/.*\n                                                ; /* ignore comments */

"/*"            { BEGIN(C_COMMENT); }                     /* ignore comments */
<C_COMMENT>"*/" { BEGIN(INITIAL); }
<C_COMMENT>\n   { }
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
"="                                     return TOKEN(T_ASSIGN);

";"                                     return TOKEN(T_SEMICOLON);
"("                                     return TOKEN(T_LPAREN);
")"                                     return TOKEN(T_RPAREN);
"{"                                     return TOKEN(T_LBLOCK);
"}"                                     return TOKEN(T_RBLOCK);
","                                     return TOKEN(T_COMMA);

"->"                                    return TOKEN(T_MACCESS);
"."                                     return TOKEN(T_FACCESS);

"=="                                    return TOKEN(T_EQ);
"!="                                    return TOKEN(T_NE);
"<"                                     return TOKEN(T_LT);
"<="                                    return TOKEN(T_LE);
">"                                     return TOKEN(T_GT);
">="                                    return TOKEN(T_GE);

"+"                                     return TOKEN(T_PLUS);
"-"                                     return TOKEN(T_MINUS);
"*"                                     return TOKEN(T_MUL);
"/"                                     return TOKEN(T_DIV);
"%"                                     return TOKEN(T_MOD);

"&&"                                    return TOKEN(T_AND);
"||"                                    return TOKEN(T_OR);
"!"                                     return TOKEN(T_NOT);

[0-9]+\.[0-9]*                          SAVE_TOKEN; return T_FLOAT;
[0-9]+                                  SAVE_TOKEN; return T_INT;
\"(\\.|[^\\"])*\"                       SAVE_TOKEN; return T_STRING;

[a-zA-Z_][a-zA-Z0-9_]*                  SAVE_TOKEN; return T_IDENTIFIER ;

.                                       printf("Unknown token!\n"); yyterminate();

%%
