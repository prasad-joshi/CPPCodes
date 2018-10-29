/*
 * Assume you have a method isSubstring which checks if one word is a
 * substring of another. Given two strings, s1 and s2, write code to
 * check if s2 is a rotation of s1 using only one call to isSubstring
 * (i.e., “waterbottle” is a rotation of “erbottlewat”)
 */

#include <string>
#include <algorithm>
#include <iostream>

bool IsSubstr(const std::string& s1, const std::string& s2) {
	return s1.find(s2) != std::string::npos;
}

bool IsRotation(const std::string& s1, const std::string& s2) {
	if (s1.size() != s2.size()) {
		return false;
	}
	std::string s1s1 = s1 + s1;
	return IsSubstr(s1s1, s2);
}

int main(int argc, char* argv[]) {
	for (int i = 1; argv[i] != nullptr; i += 2) {
		if (argv[i+1] == nullptr) {
			break;
		}
		std::string s1(argv[i]);
		std::string s2(argv[i+1]);

		bool is_rot = IsRotation(s1, s2);
		if (is_rot) {
			std::cout << s2 << " is rotation of " << s1 << std::endl;
		}
	}

	return 0;
}
