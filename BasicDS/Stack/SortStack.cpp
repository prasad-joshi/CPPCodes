/*
 * Write a program to sort a stack in ascending order. You should not make any
 * assumptions about how the stack is implemented. The following are the only
 * functions that should be used to write this program:
 * 
 * push | pop | peek | isEmpty
 */

#include <iostream>
#include <algorithm>
#include <random>

#include <cerrno>
#include "Stack.h"

void Usage(std::string pro) {
	std::cout << pro << " <nelements>" << std::endl;
}

auto GenerateRandomNumbers(int size) {
	std::vector<int> v(size);
    std::mt19937 rng;
	rng.seed(std::random_device()());
	std::generate(v.begin(), v.end(), rng);
	return v;
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

	Stack<int> s1(nelements);
	for (auto e : GenerateRandomNumbers(nelements)) {
		s1.Push(e);
	}
	std::cout << s1 << std::endl;

	Stack<int> sorted(nelements);
	while (not s1.IsEmpty()) {
		auto n = s1.Peek().value();
		s1.Pop();
		if (sorted.IsEmpty()) {
			sorted.Push(n);
			continue;
		}

		auto sorted_n = sorted.Peek().value();
		if (sorted_n <= n) {
			sorted.Push(n);
			continue;
		}

		while (not sorted.IsEmpty()) {
			sorted_n = sorted.Peek().value();
			if (sorted_n <= n) {
				break;
			}
			sorted.Pop();
			s1.Push(sorted_n);
		}

		sorted.Push(n);
	}

	std::cout << "Sorted Stack " << sorted << std::endl;
	return 0;
}
