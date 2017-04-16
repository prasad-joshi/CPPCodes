#ifndef __TOKEN_H__
#define __TOKEN_H__

#include <string>
#include <vector>

using std::string;
using std::vector;

enum class TokenClass {
	ID,
	OPERATOR,
};

enum TokenType {
	OR,
	AND,
	EQ,
	NEQ,
	LT,
	LE,
	GT,
	GE,
	UNARY,
	ID,
	KEYWORD,
};

enum class AssocType {
	LEFT,
	RIGHT,
};

struct TokenTable {
	TokenType  token;
	int        precedence;
	AssocType  associativity;
	const char *valuep;
};

class Token {
private:
	TokenType type_;
	int       precedence_;
	AssocType associativity_;
	string    value_;
public:
	Token();
	Token(TokenType type, const string &value);
	Token(const Token &t);
	void dump() const;

	bool isOperator() const {
		return type_ != ID;
	}

	bool isOperand() const {
		return type_ == ID || type_ == KEYWORD;
	}

	bool isKeyWord() const {
		return type_ == KEYWORD;
	}

	TokenType tokenType() const {
		return type_;
	}

	int precedence() const {
		return precedence_;
	}

	const string& getValue() const {
		return value_;
	}

	AssocType associativity() const {
		return associativity_;
	}
};

vector<Token> tokanize(const vector<string> &stoken, const vector<string>& keywords);

#endif
