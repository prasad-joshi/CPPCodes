#include <iostream>
#include <vector>

void Swap(int64_t* f, int64_t* s) {
	int64_t x = *f;
	*f = *s;
	*s = x;
}

ssize_t Partition(std::vector<int64_t>& vec, ssize_t start, ssize_t end) {
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

void QuickSelect(std::vector<int64_t>& vec, ssize_t start, ssize_t end,
		uint16_t index) {
	if (start > end) {
		return;
	}

	auto p = Partition(vec, start, end);
	if (p == index) {
		return;
	} else if (p > index) {
		QuickSelect(vec, start, p-1, index);
	} else {
		QuickSelect(vec, p+1, end, index);
	}
	return;
}

int main() {
	uint16_t nelements;
	std::cout << "Count of numbers: ";
	std::cin >> nelements;

	std::vector<int64_t> numbers;
	for (uint16_t i = 0; i < nelements; ++i) {
		int64_t n;
		std::cin >> n;
		numbers.emplace_back(n);
	}

	uint16_t index;
	std::cout << "find the kth smallest element (starting with 0th index) ";
	std::cin >> index;

	QuickSelect(numbers, 0, numbers.size()-1, index);
	std::cout << index << "th elements " << numbers[index] << std::endl;

	std::cout << "Current vector ";
	for (const auto& x : numbers) {
		std::cout << x << ' ';
	}
	std::cout << std::endl;
	return 0;
}
