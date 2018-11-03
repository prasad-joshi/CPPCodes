#include <iostream>
#include "RollingHash.h"

int seed = std::random_device()();

int main(int agrc, char* argv[]) {
	std::string text(argv[1]);
	RollingHash::KarpRabinHash hasher(seed, 3);

	auto out_it = text.begin();
	auto in_it = text.begin();
	auto eit = text.end();

	for (; in_it != eit; ++in_it) {
		if (std::distance(out_it, in_it) < 3) {
			hasher.Eat(*in_it);
		} else {
			hasher.Update(*out_it, *in_it);
			++out_it;
			std::cout << "Hash " << hasher.GetHashValue() << std::endl;
		}
	}
	return 0;
}
