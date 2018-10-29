#include <iostream>
#include <algorithm>
#include <vector>
#include <random>
#include <cerrno>

#include "BinaryTree.h"

auto GenerateRandomNumbers(int size) {
	std::vector<int> v(size);
    std::mt19937 rng;
	rng.seed(std::random_device()());
	std::generate(v.begin(), v.end(), [&] () mutable {
		auto no = rng() ;
		return no % 500;
	});
	return v;
}

void Usage(std::string pro) {
	std::cout << pro << " <nelements>" << std::endl;
}

int main(int argc, char* argv[]) {
	if (argc != 2) {
		Usage(argv[0]);
		return EINVAL;
	}

	int nelements = std::atoi(argv[1]);
	if (not nelements) {
		Usage(argv[0]);
		return EINVAL;
	}

	BST<int> tree;
	for (auto e : GenerateRandomNumbers(nelements)) {
		tree.Insert(e);
	}
	std::cout << tree << std::endl;
	std::cout << tree.IsBalanced() << std::endl;
	return 0;
}
