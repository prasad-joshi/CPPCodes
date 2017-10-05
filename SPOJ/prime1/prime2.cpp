#include <iostream>
#include <vector>
#include <cmath>
#include <cassert>

int64_t find_first_odd_divisible(int64_t start, int64_t end, int64_t divby) {
	assert(start % 2 == 0);
	assert(end % 2 == 0);

	for (auto no = start + 1; no <= end; no += 2) {
		if (no % divby == 0) {
			return no;
		}
	}
	return end + 1;
}

void print_prime(int64_t start, int64_t end) {
	/* make start and end even */
	auto even_start = start;
	auto even_end   = end;
	if (even_start % 2) {
		--even_start;
	}
	if (even_end % 2) {
		++end;
	}

	auto diff = even_end - even_start;
	auto nelements = diff / 2;

	std::vector<bool> notprimes(nelements, false);

	auto e = sqrt(even_end);
	for (auto s = 3; s <= e; s += 2) {
		if ((s != 3 && s % 3 == 0) ||
				(s != 5 && s % 5 == 0) ||
				(s != 7 && s % 7 == 0)) {
			continue;
		}
		auto first = find_first_odd_divisible(even_start, even_end, s);
		if (first > end) {
			continue;
		}
		auto index = (first - even_start) / 2;
		for (; first <= end; index += s, first += (s + s)) {
			if (first == s) {
				continue;
			}
			notprimes[index] = true;
		}
	}

	auto first = even_start + 1;
	if (start <= 2)
		std::cout << "2\n";
	for (const auto& e : notprimes) {
		if (not e && first != 1) {
			std::cout << first << std::endl;
		}
		first += 2;
	}
	std::cout << std::endl;
}

int main() {
	int64_t ntests;
	std::cin >> ntests;

	while (ntests--) {
		int64_t start;
		int64_t end;

		std::cin >> start >> end;

		print_prime(start, end);
	}
}
