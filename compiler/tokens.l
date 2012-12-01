%{ 
#include "../AST.h"
#include "parser.hpp"

#define SAVE_TOKEN_STRING yylval.string = new std::string(yytext, yyleng)

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
"#flags"                                return TOKEN(K_FLAGS);

"throw"                                 return TOKEN(K_THROW);
"try"                                   return TOKEN(K_TRY);
"catch"                                 return TOKEN(K_CATCH);
"as"                                    return TOKEN(K_AS);

"if"                                    return TOKEN(K_IF);
"else"                                  return TOKEN(K_ELSE);
"for"                                   return TOKEN(K_FOR);
"while"                                 return TOKEN(K_WHILE);
"="                                     return TOKEN(T_ASSIGN);

"instanceof"                            return TOKEN(K_INSTANCEOF);
"return"                                return TOKEN(K_RETURN);
"this"                                  return TOKEN(K_THIS);
"null"                                  return TOKEN(K_NULL);
"new"                                   return TOKEN(K_NEW);

"true"                                  return TOKEN(K_TRUE);
"false"                                 return TOKEN(K_FALSE);

";"                                     return TOKEN(T_SEMICOLON);
"("                                     return TOKEN(T_LPAREN);
")"                                     return TOKEN(T_RPAREN);
"{"                                     return TOKEN(T_LBLOCK);
"}"                                     return TOKEN(T_RBLOCK);
","                                     return TOKEN(T_COMMA);

"->"                                    return TOKEN(T_MACCESS);
"."                                     return TOKEN(T_FACCESS);
"@"                                     return TOKEN(T_STATIC);

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

[0-9]+\.[0-9]*                          SAVE_TOKEN_STRING; return C_FLOAT;
[0-9]+                                  SAVE_TOKEN_STRING; return C_INTEGER;
0[xX][0-9a-fA-F]+                       SAVE_TOKEN_STRING; return C_HEX_INTEGER;
\"(\\.|[^\\"])*\"                       SAVE_TOKEN_STRING; return T_STRING;

[a-zA-Z_][a-zA-Z0-9_]*                  SAVE_TOKEN_STRING; return T_IDENTIFIER;

.                                       printf("Unknown token!\n"); yyterminate();

%%
