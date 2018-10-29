/*
 * Write a method to replace all spaces in a string with ‘%20’.
 */

#include <iostream>
#include <string>
#include <algorithm>

std::string Replace(std::string str, const char replace, std::string with) {
	std::reverse(with.begin(), with.end());
	auto nchars = std::count(str.begin(), str.end(), replace);
	auto sz = str.size();
	str.resize(sz + (nchars * (with.size() - 1)));

	auto rit = std::next(str.begin(), sz-1);
	auto wit = std::prev(str.end());
	auto bit = str.begin();

	for (; bit <= rit; --rit) {
		if (*rit != replace) {
			*wit = *rit;
			--wit;
		} else {
			for (const char c : with) {
				*wit = c;
				--wit;
			}
			if (--nchars == 0) {
				break;
			}
		}
	}

	return str;
}

int main(int argc, char* argv[]) {
	for (auto i = 1; argv[i] != nullptr; ++i) {
		std::string str(argv[i]);
		auto result = Replace(str, ' ', "%20");
		std::cout << "'" << str << "'" << " changed to " << result << std::endl;
	}
	return 0;
}
