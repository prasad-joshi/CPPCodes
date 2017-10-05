#include <iostream>
#include <vector>
#include <algorithm>

int main() {
	int64_t tests;
	std::vector<std::string> strings;

	std::cin >> tests;

	while (tests--) {
		int64_t nstrings;
		std::cin >> nstrings;

		strings.reserve(nstrings);

		while (nstrings--) {
			std::string line;
			std::cin >> line;
			strings.emplace_back(std::move(line));
		}

		std::sort(strings.begin(), strings.end());

		for (auto&& e : strings) {
			std::cout << e << std::endl;
		}
		strings.clear();
	}
}
