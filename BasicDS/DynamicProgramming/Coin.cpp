#include <iostream>
#include <iomanip>
#include <set>
#include <map>
#include <vector>
#include <algorithm>
#include <cassert>

using CoinTable = std::vector<std::vector<uint32_t>>;

void DumpCoinTable(const std::vector<uint32_t> denoms, const CoinTable& table) {
	auto it = denoms.begin();
	for (const auto& d : table) {
		std::cout << *it++ << ": ";
		for (const auto v : d) {
			std::cout << std::setw(4) << v << " ";
		}
		std::cout << "\n";
	}
	std::cout << std::endl;
}

CoinTable BuildCoinTable(const std::vector<uint32_t> denoms,
		const uint32_t value) {
	CoinTable table;

	for (uint16_t d : denoms) {
		(void) d;
		std::vector<uint32_t> counts(value+1, 0);
		table.emplace_back(std::move(counts));
	}

	for (int d = 0; d < table.size(); ++d) {
		for (int c = 1; c < table[d].size(); ++c) {
			auto prev_d = d - 1;
			uint32_t prev_c = value+1;
			if (prev_d >= 0) {
				prev_c = table[prev_d][c];
			}

			uint32_t min_c = 0;
			if (c < denoms[d]) {
				min_c = prev_c;
			} else {
				min_c = std::min(prev_c, table[d][c - denoms[d]] + 1);
			}

			table[d][c] = min_c;
		}
	}

	DumpCoinTable(denoms, table);
	return table;
}

std::vector<uint32_t> GetCoins(const std::vector<uint32_t>& denoms,
		const CoinTable& table, const uint32_t value) {
	std::vector<uint32_t> result;

	if (table[denoms.size() - 1][value] > value) {
		return result;
	}

	int d_index = denoms.size() - 1;
	uint32_t v = value;

	while (v) {
		if (d_index > 0 and table[d_index][v] == table[d_index-1][v]) {
			--d_index;
			continue;
		}

		auto denom = denoms[d_index];
		result.emplace_back(denom);
		assert(v >= denom);
		v -= denom;
	}

	return result;
}

int main() {
	int ndenoms;

	std::set<uint32_t> denoms_set;
	std::cout << "Enter number of denominations ";
	std::cin >> ndenoms;
	std::cout << "Enter denominations ";
	for (auto i = ndenoms; i > 0; --i) {
		uint16_t d;
		std::cin >> d;
		denoms_set.emplace(d);
	}
	std::vector<uint32_t> denoms(denoms_set.begin(), denoms_set.end());

	std::cout << "Enter value ";
	uint32_t value;
	std::cin >> value;
	std::cout << std::endl;


	std::cout << "Denominations ";
	for (const auto& d : denoms) {
		std::cout << d << " ";
	}
	std::cout << "\n";
	std::cout << "Value " << value << std::endl;

	auto table = BuildCoinTable(denoms, value);
	auto coins = GetCoins(denoms, table, value);
	if (coins.empty()) {
		std::cout << "Cannot give change";
	} else {
		std::sort(coins.begin(), coins.end());
		std::cout << "Coins: ";
		for (const auto& c : coins) {
			std::cout << c << " ";
		}
	}
	std::cout << std::endl;
	return 0;
}
