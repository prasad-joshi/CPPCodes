#include <iostream>
#include <vector>

void initialize_not_primes(std::vector<bool>& notprimes, int64_t end) {
	for (auto s = 2; s <= end; s += 2) {
		notprimes[s] = true;
	}

	for (auto s = 3; s <= end; s += 3) {
		notprimes[s] = true;
	}

	for (auto s = 5; s <= end; s += 5) {
		notprimes[s] = true;
	}

	for (auto s = 7; s <= end; s += 7) {
		notprimes[s] = true;
	}

	notprimes[0] = true;
	notprimes[1] = true;
	notprimes[2] = false;
	notprimes[3] = false;
	notprimes[5] = false;
	notprimes[7] = false;
}

void set_all_multiples(std::vector<bool>& notprimes, int64_t s, int64_t end) {
	if (notprimes[s]) {
		return;
	}
	for (auto i = s + s; i<= end; i += s) {
		notprimes[i] = true;
	}
}

void print_prime(int64_t start, int64_t end) {
	std::vector<bool> notprimes(end+1, false);

	initialize_not_primes(notprimes, end);

	for (auto s = 9; s <= end; s += 2) {
		set_all_multiples(notprimes, s, end);
	}

	for (auto s = start; s <= end; ++s) {
		if (not notprimes[s]) {
			std::cout << s << std::endl;
		}
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
