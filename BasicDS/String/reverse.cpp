/*
 * Write code to reverse a C-Style String
 */

#include <string>
#include <iostream>

void Reverse(std::string& str) {
	auto it1 = str.begin();
	auto it2 = std::prev(str.end());

	for (; it1 < it2; ++it1, --it2) {
		auto c1 = *it1;
		*it1 = *it2;
		*it2 = c1;
	}
}

int main(int argc, char* argv[]) {
	for (auto i = 1; argv[i] != nullptr; ++i) {
		std::string str(argv[i]);
		Reverse(str);
		std::cout << "Reverse of " << argv[1] << " is " << str << std::endl;
	}
	return 0;
}
