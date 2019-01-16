#include <string>
#include <iostream>
#include <stdexcept>

#include <cctype>
#include <cmath>
#include <cassert>

enum class TokenType {
	kEof,
	kInteger,
	kPlus,
	kMinus,
	kMultiply,
	kDivide,
	kMod,
	kPower,
};

std::ostream& operator << (std::ostream& os, TokenType rhs) {
	switch (rhs) {
	case TokenType::kEof:
		os << "eof";
		break;
	case TokenType::kInteger:
		os << "integer";
		break;
	case TokenType::kPlus:
		os << "plus";
		break;
	case TokenType::kMinus:
		os << "minus";
		break;
	case TokenType::kMultiply:
		os << "mul";
		break;
	case TokenType::kDivide:
		os << "div";
		break;
	case TokenType::kMod:
		os << "mod";
		break;
	case TokenType::kPower:
		os << "power";
		break;
	}
	return os;
}

class Token {
public:
	Token(TokenType type, std::string&& value) :
		value_(std::forward<std::string>(value)), type_(type) {
	}

	Token(TokenType type) : type_(type) {
	}

	Token(TokenType type, char value) : value_(1, value), type_(type) {
	}

	const std::string& Value() const noexcept {
		return value_;
	}

	TokenType GetTokenType() const noexcept {
		return type_;
	}

	friend std::ostream& operator << (std::ostream& os, const Token& rhs) {
		return os << "Token({" << rhs.type_ << "}, {" << rhs.value_ << "})";
	}
private:
	std::string value_;
	TokenType type_;
};

class Interpretor {
public:
	Interpretor(std::string&& text) : text_(std::forward<std::string>(text)),
			it_(text_.begin()), end_(text_.end()) {
	}

	int32_t Expr() {
		current_.token_ = NextToken();
		assert(current_.token_.GetTokenType() == TokenType::kInteger);
		int32_t result = std::stoi(current_.token_.Value());
		Eat(current_.token_.GetTokenType());

		while (current_.token_.GetTokenType() != TokenType::kEof) {
			auto op = current_.token_;
			Eat(op.GetTokenType());

			auto right = current_.token_;
			assert(right.GetTokenType() == TokenType::kInteger);
			Eat(right.GetTokenType());
			auto v = std::stoi(right.Value());

			switch (op.GetTokenType()) {
			default:
				assert(0);
				return -EINVAL;
			case TokenType::kPlus:
				result += v;
				break;
			case TokenType::kMinus:
				result -= v;
				break;
			case TokenType::kMultiply:
				result *= v;
				break;
			case TokenType::kDivide:
				result /= v;
				break;
			case TokenType::kMod:
				result %= v;
				break;
			case TokenType::kPower:
				result = std::pow(result, v);
				break;
			}
		}
		return result;
	}

private:
	void Error() {
		std::string error("Unknown token: ");
		error += *it_;
		throw std::runtime_error(std::move(error));
	}

	Token NextToken() {
		while (it_ != end_ and std::isspace(*it_)) {
			++it_;
		}
		if (it_ == end_) {
			return Token(TokenType::kEof);
		}

		if (std::isdigit(*it_)) {
			auto begin = it_;
			while (it_ != end_ and std::isdigit(*it_)) {
				++it_;
			}
			std::string v(begin, it_);
			return Token(TokenType::kInteger, std::move(v));
		}

		switch (*it_) {
		case '+':
			++it_;
			return Token(TokenType::kPlus);
		case '-':
			++it_;
			return Token(TokenType::kMinus);
		case '*':
			++it_;
			return Token(TokenType::kMultiply);
		case '/':
			++it_;
			return Token(TokenType::kDivide);
		case '%':
			++it_;
			return Token(TokenType::kMod);
		case '^':
			++it_;
			return Token(TokenType::kPower);
		defult:
			Error();
		}
		return current_.token_;
	}

	void Eat(TokenType type) {
		if (type != current_.token_.GetTokenType()) {
			Error();
		}
		current_.token_ = NextToken();
	}
private:
	std::string text_;
	std::string::iterator it_{};
	std::string::iterator end_{};

	struct {
		Token token_{TokenType::kEof};
	} current_;
};

int main() {
	while (true) {
		std::string input;
		std::cout << "calc> ";
		std::getline(std::cin, input);
		if (input.empty()) {
			std::cout << "DONE" << std::endl;
			break;
		}

		Interpretor intpretor(std::move(input));
		std::cout << intpretor.Expr() << std::endl;
	}
}
