#include <iostream>
#include <string>
#include <cassert>

#include "Token.h"

using std::string;
using std::cout;
using std::endl;

struct TokenTable table[] = {
	{OR, 1, AssocType::LEFT, "||"},
	{AND, 2, AssocType::LEFT, "&&"},
	{EQ, 3, AssocType::LEFT, "=="},
	{NEQ, 3, AssocType::LEFT, "!="},
	{LT, 4, AssocType::LEFT, "<"},
	{LE, 4, AssocType::LEFT, "<="},
	{GT, 4, AssocType::LEFT, ">"},
	{GE, 4, AssocType::LEFT, ">="},
	{UNARY, 5, AssocType::RIGHT, "-"},
	{ID, 6, AssocType::LEFT, ""},
	{KEYWORD, 6, AssocType::LEFT, ""},
};

int getTokenPrecedence(TokenType token) {
	return table[token].precedence;
}

AssocType getTokenAssociativity(TokenType token) {
	return table[token].associativity;
}

const char *getTokenValue(TokenType token) {
	return table[token].valuep;
}

TokenType stringToToken(const string &e) {
	TokenType type;

	if (e.length() == 1) {
		switch (e[0]) {
		case '<':
			type = LT;
			break;
		case '>':
			type = GT;
			break;
		case '-':
			type = UNARY;
			break;
		default:
			type = ID;
			break;
		}
	} else if (e.length() == 2) {
		switch (e[0]) {
		case '<':
			assert(e[1] == '=');
			type = LE;
			break;
		case '>':
			assert(e[1] == '=');
			type = GE;
			break;
		case '=':
			assert(e[1] == '=');
			type = EQ;
			break;
		case '!':
			assert(e[1] == '=');
			type = NEQ;
			break;
		case '&':
			assert(e[1] == '&');
			type = AND;
			break;
		case '|':
			assert(e[1] == '|');
			type = OR;
			break;
		default:
			type = ID;
		}
	} else {
		type = ID;
	}

	return type;
}

vector<Token> tokanize(const vector<string> &stoken, const vector<string>& keywords) {
	vector<Token> rc;

	for (const auto &e : stoken) {
		auto t = stringToToken(e);
		switch (t) {
		case ID:
			for (const auto &k : keywords) {
				if (k == e) {
					t = KEYWORD;
					break;
				}
			}
			break;
		default:
			break;
		}
		Token x(t, e);
		rc.emplace_back(x);
	}

	return rc;
}

Token::Token() : Token(ID, string("")) {
}

Token::Token(TokenType type, const string &value) : type_(type), value_(value) {
	precedence_    = getTokenPrecedence(type_);
	associativity_ = getTokenAssociativity(type_);
}

Token::Token(const Token &t) {
	type_          = t.type_;
	precedence_    = t.precedence_;
	associativity_ = t.associativity_;
	value_         = t.value_;
}

void Token::dump() const {
	cout << value_ + " " + std::to_string(precedence_) << endl;
}
