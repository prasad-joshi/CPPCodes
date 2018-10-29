/*
 * Implement an algorithm to determine if a string has all unique characters.
 * What if you can not use additional data structures?
 */

#include <string>
#include <bitset>
#include <iostream>

int ToInt(const char c) {
	return c;
}

bool HasUniqueCharacters(const std::string& str) {
	auto it = str.begin();
	auto eit = str.end();
	std::bitset<256> bits;

	for (; it != eit; ++it) {
		auto v = ToInt(*it);
		if (bits.test(v)) {
			return false;
		}
		bits.set(v);
	}
	return true;
}

int main(int argc, char* argv[]) {
	for (auto i = 1; argv[i] != nullptr; ++i) {
		std::string str(argv[i]);
		auto unique = HasUniqueCharacters(str);
		if (unique) {
			std::cout << str << " has unique characters" << std::endl;
		} else {
			std::cout << str << " has repeated characters" << std::endl;
		}
	}
	return 0;
}
