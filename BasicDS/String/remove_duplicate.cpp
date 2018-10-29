/*
 * Design an algorithm and write code to remove the duplicate characters in a
 * string without using any additional buffer. NOTE: One or two additional
 * variables are fine.  An extra copy of the array is not.
 *
 * Write the test cases for this method.
 */

#include <iostream>
#include <string>
#include <bitset>
#include <algorithm>
#include <cassert>

int ToInt(const char c) {
	return c;
}

void RemoveDuplicateNoBuffer(std::string& str) {
	auto wit = str.begin();
	auto rit = wit;
	auto eit = str.end();

	for (; rit != eit; ++rit) {
		auto it = std::find(str.begin(), wit, *rit);
		if (it == wit) {
			*wit = *rit;
			++wit;
		}
	}
	str.erase(wit, eit);
}

void RemoveDuplicate(std::string& str) {
	auto wit = str.begin();
	auto rit = wit;
	auto eit = str.end();
	std::bitset<256> bits;

	for (; rit != eit; ++rit) {
		auto v = ToInt(*rit);
		if (not bits.test(v)) {
			*wit = *rit;
			++wit;
			bits.set(v);
		}
	}
	str.erase(wit, eit);
}

int main(int argc, char* argv[]) {
	for (auto i = 1; argv[i] != nullptr; ++i) {
		std::string s1(argv[i]);
		RemoveDuplicateNoBuffer(s1);

		std::string s2(argv[i]);
		RemoveDuplicate(s2);

		std::cout << argv[1] << " removed duplicate "
			<< s1 << " " << s2 << std::endl;
		assert(s1 == s2);
	}
	return 0;
}
