#include <iostream>

#include "node.h"

using std::cout;
using std::endl;

extern NProgram *programp;
extern int yyparse();

int main(int argc, char **argv)
{
	yyparse();

	cout << programp << endl;
	programp->dump();
	cout << endl;

	return 0;
}