#ifndef __PARSER_H__
#define __PARSER_H__

#include <vector>
#include <string>

#include "Token.h"

using std::vector;
using std::string;

vector<Token> recursive_decent(const string &expr, const vector<string>& keywords);

#endif
