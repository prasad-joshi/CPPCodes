#include <iostream>
#include <cmath>

int64_t divsum(int64_t const& number) {
	switch (number) {
	case 0:
	case 1:
		return 0;
	case 2:
		return 1;
	case 3:
		return 1;
	}

	int64_t sum = 1;
	int64_t end = sqrt(number);
	for (auto i = 2; i <= end; ++i) {
		if (number % i == 0) {
			sum += i;
			auto d = number / i;
			if (i != d) {
				sum += d;
			}
		}
	} 
	return sum;
}

#if 1

int main() {
	int64_t nlines;

	std::cin >> nlines;

	while (nlines--) {
		int64_t number;
		std::cin >> number;
		auto sum = divsum(number);
		std::cout << sum << std::endl;
	}
}

#else

#include <cassert>

int divsum2(int64_t const& number) {
	int64_t sum = 0;
	for (auto i = 1; i <= number / 2; ++i) {
		if (number % i == 0) {
			sum += i;
		}
	}

	return sum;
}


int main() {
	for (auto i = 0; i <= 500000; i++) {
		if (divsum(i) != divsum2(i)) {
			std::cout << i << "," << divsum(i) << "," << divsum2(i) << std::endl;
			assert(0);
		}
	}
}
#endif
