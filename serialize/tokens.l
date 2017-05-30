%{

#include <string>
#include <iostream>
#include <memory>

#include "node.h"
#include "parser.hpp"

inline yytokentype save_string_token(yytokentype token) {
	std::cout << "S " << yytext << " " << token << std::endl;
	yylval.stringp = new std::string(yytext, yyleng);
	return token;
}

inline yytokentype save_token(yytokentype token) {
	std::cout << "T " << yytext << " " << token << std::endl;
	yylval.token = token;
	return token;
}

%}

%option noyywrap

%%

[ \t\n]                 ;
"="                     return save_token(TEQUAL);
":"                     return save_token(TCOLON);
";"                     return save_token(TSEMICOLON);
"{"                     return save_token(TBRACEOPEN);
"}"                     return save_token(TBRACECLOSE);
"using"                 return save_token(TUSING);
"struct"                return save_token(TSTRUCT);

[a-zA-Z_][a-zA-Z0-9_]*  return save_string_token(TIDENTIFIER);
[0-9]+                  return save_string_token(TINTEGER);

.                       printf("Unknown token!n"); yyterminate();

%%