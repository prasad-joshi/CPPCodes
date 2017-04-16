#include <iostream>
#include <vector>
#include <string>
#include <cassert>

#include "Stack.h"
#include "parser.h"

using std::vector;
using std::string;
using std::cout;
using std::endl;

static vector<string> split(const string &line, const char &delim) {
	vector<string> v;
	size_t         e;
	size_t         s = 0;

	while ((e = line.find(delim, s)) != string::npos) {
		if (s != e) {
			v.emplace_back(line.substr(s, e-s));
		}
		s = e + 1;
	}

	if (s != e) {
		v.emplace_back(line.substr(s));
	}

	return v;
}

vector<Token> recursive_decent(const string &expr, const vector<string>& keywords) {
	auto stoken = split(expr, ' ');
	auto tokens = tokanize(stoken, keywords);

	vector<Token> outQ;
	Stack<Token>  stack;
	for (const auto &e: tokens) {
		if (e.isOperand()) {
			outQ.emplace_back(e);
			continue;
		}
		assert(e.isOperator());

		bool done = false;
		while (!done && !stack.isEmpty()) {
			auto top = stack.lookupTop();
			switch (e.associativity()) {
			case AssocType::LEFT:
				if (e.precedence() <= top.precedence()) {
					outQ.emplace_back(stack.pop());
				} else {
					done = true;
				}
				break;
			case AssocType::RIGHT:
				if (e.precedence() < top.precedence()) {
					outQ.emplace_back(stack.pop());
				} else {
					done = true;
				}
				break;
			}
		}
		stack.push(e);
	}

	while (!stack.isEmpty()) {
		outQ.emplace_back(stack.pop());
	}
	return outQ;
}

#if 0
int main() {
	string q("T < 20 || H > 30 && T < 20 || H < 20");
	auto postfix = recursive_decent(q);

	for (const auto &e : postfix) {
		cout << e.getValue() << " ";
	}
	cout << endl;

	return 0;
}
#endif
