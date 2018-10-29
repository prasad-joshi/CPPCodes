/*
 * Given a sorted (increasing order) array, write an algorithm to create a
 * binary tree with minimal height.
 */
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

void InsertSorted(BinaryTree<int>& tree, std::vector<int>& nos, ssize_t left, ssize_t right) {
	if (left > right) {
		return;
	}
	size_t mid = (left + right) / 2;
	tree.Insert(nos[mid]);
	InsertSorted(tree, nos, left, mid-1);
	InsertSorted(tree, nos, mid+1, right);
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

	auto nos = GenerateRandomNumbers(nelements);
	std::sort(nos.begin(), nos.end());

	BinaryTree<int> tree;
	InsertSorted(tree, nos, 0, nos.size() - 1);
	std::cout << tree << std::endl;
	return 0;
}
