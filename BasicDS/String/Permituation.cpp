#include <iostream>
#include <vector>
#include <string>

void Permute(std::vector<std::string>& result, std::string& chars, int start, int end) {
	if (start == end) {
		result.emplace_back(chars);
		return;
	}

	for (auto s = start; s <= end; ++s) {
		std::swap(chars[s], chars[start]);
		Permute(result, chars, start + 1, end);
		std::swap(chars[s], chars[start]);
	}
}

int main(int argc, char* argv[]) {
	std::string chars(argv[1]);
	std::vector<std::string> result;
	Permute(result, chars, 0, chars.size() - 1);
	for (const auto& s : result) {
		std::cout << s << ' ';
	}
	std::cout << std::endl;
	return 0;
}
