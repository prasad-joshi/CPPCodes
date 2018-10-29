/*
 * Write a method to decide if two strings are anagrams or not.
 */

#include <string>
#include <algorithm>
#include <iostream>
#include <vector>
#include <cassert>

bool IsAnagramUsingSort(std::string s1, std::string s2) {
	std::sort(s1.begin(), s1.end());
	std::sort(s2.begin(), s2.end());
	return s1 == s2;
}

int ToInt(const char c) {
	return c;
}

std::vector<int> GetCharacterCount(const std::string& str) {
	std::vector<int> counts(256, 0);
	for (const auto c : str) {
		auto v = ToInt(c);
		++counts[v];
	}
	return counts;
}

bool IsAnangram(std::string s1, std::string s2) {
	auto s1c = GetCharacterCount(s1);
	auto s2c = GetCharacterCount(s2);
	return std::equal(s1c.begin(), s1c.end(), s2c.begin());
}

int main(int argc, char* argv[]) {
	for (auto i = 1; argv[i] != nullptr and argv[i+1] != nullptr; i += 2) {
		std::string s1(argv[i]);
		std::string s2(argv[i+1]);
		auto anagram = IsAnagramUsingSort(s1, s2);
		assert(anagram == IsAnangram(s1, s2));
		std::cout << "String " << s1 << "," << s2;
		if (anagram) {
			std::cout << " are anagram";
		} else {
			std::cout << " are not anagram";
		}
		std::cout << std::endl;
	}
	return 0;
}
