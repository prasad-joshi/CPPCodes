#include <iostream>
#include <vector>
#include <cstdlib>

template <typename T>
void Swap(T* f, T* s) {
	T x = std::move(*f);
	*f = std::move(*s);
	*s = std::move(x);
}

template <typename T>
ssize_t Parition(std::vector<T>& vec, ssize_t start, ssize_t end) {
	const int64_t pivot = vec[end];
	ssize_t first_high = start;

	for (ssize_t i = start; i < end; ++i) {
		if (vec[i] > pivot) {
			continue;
		}

		Swap(&vec[i], &vec[first_high]);
		++first_high;
	}
	Swap(&vec[first_high], &vec[end]);
	return first_high;
}

template <typename T>
void QuickSort(std::vector<T>& vec, ssize_t start, ssize_t end) {
	if (start >= end) {
		return;
	}

	auto p = Parition(vec, start, end);
	QuickSort(vec, start, p-1);
	QuickSort(vec, p+1, end);
}

int main(int argc, char* argv[]) {
	std::vector<int64_t> numbers;
	for (int i = 1; argv[i]; ++i) {
		auto n = std::atoll(argv[i]);
		numbers.emplace_back(n);
	}

	QuickSort(numbers, 0, numbers.size() - 1);

	for (const auto& n : numbers) {
		std::cout << n << ' ';
	}
	std::cout << std::endl;
	return 0;
}
