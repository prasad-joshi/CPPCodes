#include <iostream>
#include <iomanip>
#include <set>
#include <vector>
#include <algorithm>
#include <cassert>

struct DPCoinResult {
	DPCoinResult(std::vector<uint32_t> t, std::vector<uint32_t> p) :
			coins(t), predecessors(p) {
	
	}
	std::vector<uint32_t> coins;
	std::vector<uint32_t> predecessors;
};

DPCoinResult DPCoin(const std::vector<uint32_t>& denoms,
		uint32_t value) {
	const uint32_t kMaxCoins = value + 1;
	std::vector<uint32_t> coins(value+1, 0);
	std::vector<uint32_t> predecessors(value+1, 0);

	for (uint32_t v = 1; v <= value; ++v) {
		uint32_t pred = v + 1;
		uint32_t count = kMaxCoins;
		for (uint32_t j = 0; j < denoms.size(); ++j) {
			auto d = denoms[j];
			if (v < d) {
				continue;
			}

			auto c = coins[v - d] + 1;
			if (count > c) {
				count = c;
				pred = v - d;
			}
		}

		coins[v] = count;
		predecessors[v] = pred;
	}

	return DPCoinResult(std::move(coins), std::move(predecessors));
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

	auto result = DPCoin(denoms, value);

	for (const auto c : result.coins) {
		std::cout << c << " ";
	}
	std::cout << '\n';

	for (const auto p : result.predecessors) {
		std::cout << p << ' ';
	}
	std::cout << '\n';

	std::vector<uint32_t> coins;
	auto index = *result.predecessors.rbegin();
	std::cout << index << ' ';
	while (index) {
		index = result.predecessors[index];
		std::cout << index << ' ';
	}
	std::cout << '\n';
	return 0;
}
