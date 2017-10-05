/*
Peter wants to generate some prime numbers for his cryptosystem. Help him! Your
task is to generate all prime numbers between two given numbers!

Input

The input begins with the number t of test cases in a single line (t<=10). In
each of the next t lines there are two numbers m and n
(1 <= m <= n <= 1000000000, n-m<=100000) separated by a space.

Output

For every test case print all prime numbers p such that m <= p <= n, one number
per line, test cases separated by an empty line.

Example

Input:
2
1 10
3 5

Output:
2
3
5
7

3
5
*/

#include <iostream>
#include <vector>
#include <algorithm>
#include <limits>
#include <cmath>
#include <cassert>

void compute_all_primes(std::vector<int64_t>& result, int64_t max) {
	int64_t e = sqrt(max) + 1;
	std::vector<bool> primes(e, true);

	primes[0] = false;
	primes[1] = false;
	for (auto s = 2; s < e; ++s) {
		if ((s != 2 && s % 2 == 0) ||
				(s != 3 && s % 3 == 0) ||
				(s != 5 && s % 5 == 0) ||
				(s != 7 && s % 7 == 0)) {
			continue;
		}
		auto sz = primes.size();
		for (auto i = s + s; i < sz; i += s) {
			primes[i] = false;
		}
	}

	for (auto s = 0; s < e; ++s) {
		if (primes[s] == true) {
			result.emplace_back(s);
		}
	}
}

int64_t find_first_odd_divisible(int64_t start, int64_t end, int64_t divby) {
	assert(start % 2 == 0);
	assert(end % 2 == 0);

	for (auto no = start; no <= end; ++no) {
		if (no % divby == 0) {
			return no;
		}
	}
	return end + 1;
}


void print_prime(const std::vector<int64_t>& basic_primes, int64_t start,
		int64_t end) {
	int64_t even_start = start;
	if (even_start % 2) {
		even_start = start - 1;
	}

	int64_t even_end = end;
	if (even_end % 2) {
		even_end = end + 1;
	}
	auto sq = sqrt(even_end);

	auto diff = even_end - even_start;
	std::vector<bool> primes(diff, true);

	for (const auto& prime : basic_primes) {
		if (prime > sq) {
			break;
		}
		auto cur = find_first_odd_divisible(even_start, even_end, prime);
		if (cur > end) {
			continue;
		}
		auto index = cur - even_start;
		for (; cur <= end; index += prime, cur += prime) {
			if (cur == prime) {
				continue;
			}
			primes[index] = false;
		}
	}

	size_t sz = primes.size();
	auto cur = even_start;
	for (const auto& e : primes) {
		if (e and cur != 1 and cur >= start and cur <= end) {
			std::cout << cur << std::endl;
		}
		++cur;
	}
	std::cout << std::endl;
}

int main() {
	int64_t ntests;
	std::cin >> ntests;

	int64_t max_end = std::numeric_limits<int64_t>::min();
	std::vector<std::pair<int64_t, int64_t>> test_cases;

	while (ntests--) {
		int64_t start;
		int64_t end;

		std::cin >> start >> end;
		test_cases.emplace_back(start, end);
		max_end = std::max(max_end, end);
	}

	std::vector<int64_t> primes;
	compute_all_primes(primes, max_end);

	for (const auto& e : test_cases) {
		print_prime(primes, e.first, e.second);
	}
	return 0;
}
