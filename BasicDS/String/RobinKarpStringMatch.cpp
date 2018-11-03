#include <iostream>
#include "RollingHash.h"

int seed = std::random_device()();

uint32_t GetStringHash(const std::string& str) {
	RollingHash::KarpRabinHash hasher(seed, str.size());
	for (const auto c : str) {
		hasher.Eat(c);
	}
	return hasher.GetHashValue();
}

int main(int agrc, char* argv[]) {
	std::string text(argv[1]);
	std::string match(argv[2]);

	auto match_hash = GetStringHash(match);

	RollingHash::KarpRabinHash hasher(seed, match.size());

	auto in_index = 0;
	for (; in_index < match.size(); ++in_index) {
		hasher.Eat(text[in_index]);
	}
	auto out_index = 0;

	for (; in_index <= text.size(); ++in_index, ++out_index) {
		if (hasher.GetHashValue() == match_hash) {
			bool found = true;
			for (auto i = out_index, j = 0; i < in_index; ++i, ++j) {
				if (text[i] != match[j]) {
					found = false;
					break;
				}
			}

			if (found) {
				std::cout << "Found match at "
					<< out_index
					<< std::endl;
			}
		}

		if (in_index < text.size()) {
			hasher.Update(text[out_index], text[in_index]);
		}
	}
	return 0;
}
