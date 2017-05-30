%{
	#include <string>
	#include <memory>
	#include <vector>

	#include <cstdio>
	#include <cstdlib>

	#include "node.h"

	NProgram *programp;

	extern int yylex();

	void yyerror(const char *s) {
		std::printf("Error: %s\n", s);
		std::exit(1);
	}
%}

/* Represents the many different ways we can access our data */
%union {
	NIdentifier              *identp;
	NVariable                *varp;
	std::vector<NVariable*>  *varvecp;
	NStatement               *stmtp;
	std::vector<NStatement*> *stmtvecp;
	std::string              *stringp;
	int                      token;
}

/*
 * Define our terminal symbols (tokens). This should
 * match our tokens.l lex file. We also define the node type
 * they represent.
 */
%token <stringp> TIDENTIFIER TINTEGER TDOUBLE
%token <token>  TSEMICOLON TBRACEOPEN TBRACECLOSE
%token <token>  TEQUAL TCOLON TUSING TSTRUCT
%token <token>  TTYPEI8 TTYPEI16 TTYPEI32 TTYPEI64 TTYPEDOUBLE TTYPECHAR TTYPEBOOL

/*
 * Define the type of node our nonterminal symbols represent.
 * The types refer to the %union declaration above. Ex: when
 * we call an ident (defined by union type ident) we are really
 * calling an (NIdentifier*). It makes the compiler happy.
 */
%type <identp>    ident using_type
%type <varp>      struct_block_stmt
%type <varvecp>   struct_block
%type <stmtp>     struct_stmt using_stmt stmt
%type <stmtvecp>  stmts

%start program

%%

program : stmts { programp = new NProgram($1); }
		;

stmts : stmt TSEMICOLON { $$ = new std::vector<NStatement *>{$1}; }
	  | stmts stmt TSEMICOLON { $1->emplace_back($2); }
	  ;

stmt : using_stmt  { $$ = $1; }
	 | struct_stmt { $$ = $1; }
	 ;

using_stmt : TUSING ident TEQUAL using_type { $$ = new NUsingStatement($2, $4); }
		   ;

using_type : ident { $$ = $1; }
		   | TSTRUCT ident { $$ = $2; }
		   ;

struct_stmt : TSTRUCT ident TBRACEOPEN struct_block TBRACECLOSE { $$ = new NStructStatement($2, $4); }
			;

struct_block : struct_block_stmt TSEMICOLON { $$ = new std::vector<NVariable *>{$1}; }
			 | struct_block struct_block_stmt TSEMICOLON { $1->emplace_back($2); }
			 | %empty { $$ = new std::vector<NVariable *>(); }
			 ;

struct_block_stmt : TINTEGER TCOLON ident ident { $$ = new NVariable($4, $3, $1); }
				  | TINTEGER TCOLON TSTRUCT ident ident { $$ = new NVariable($5, $4, $1); }
				  ;

ident : TIDENTIFIER { $$ = new NIdentifier(*$1); delete $1; }
	  ;
%%