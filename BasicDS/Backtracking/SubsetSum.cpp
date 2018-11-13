/*
 * given a set of positive integers and an integer sum, is there non-empty
 * subset whose sum is <sum>.
 *
 * for example:
 * s = {7, 3, 2, 5, 8} and Sum = 14
 * subset = {7, 2, 5}
 */

#include <vector>
#include <iostream>
#include <algorithm>
#include <numeric>
#include <cassert>

class SubsetSum {
public:
	SubsetSum(std::vector<uint32_t> numbers, uint32_t sum) :
			numbers_(std::move(numbers)), sum_(sum) {
		/* rearrange numbers in increasing order */
		std::sort(numbers_.begin(), numbers_.end());
	}

	void InsertResult(std::vector<bool>& bitmap, int index) {
		std::vector<uint32_t> r;
		uint32_t sum = 0;
		for (int i = 0; i <= index; ++i) {
			assert(sum <= sum_);
			if (bitmap[i]) {
				r.emplace_back(numbers_[i]);
				sum += numbers_[i];
			}
		}
		assert(sum == sum_);
		result_.emplace_back(std::move(r));
	}

	void Compute(std::vector<bool>& bitmap, uint32_t current_sum, int index,
			uint32_t pending) {
		auto x = numbers_[index];
		pending -= x;

		bitmap[index] = true;
		if (current_sum + x == sum_) {
			InsertResult(bitmap, index);
		} else if (current_sum + x + numbers_[index + 1] <= sum_) {
			Compute(bitmap, current_sum + x, index+1, pending);
		}

		if (current_sum + pending < sum_ or
				current_sum + numbers_[index + 1] > sum_) {
			/* bounding function */
			return;
		}

		bitmap[index] = false;
		Compute(bitmap, current_sum, index + 1, pending);
	}

	void Compute() {
		if (not result_.empty()) {
			return;
		}
		auto s = std::accumulate(numbers_.begin(), numbers_.end(), 0);
		if (s < sum_) {
			return;
		}
		std::vector<bool> bitmap(numbers_.size(), false);
		Compute(bitmap, 0, 0, s);
	}

	const std::vector<std::vector<uint32_t>>& GetResult() const noexcept {
		return result_;
	}
private:
	std::vector<uint32_t> numbers_;
	const uint32_t sum_;
	std::vector<std::vector<uint32_t>> result_;
};

int main(int argc, char* argv[]) {
	uint32_t nelements;
	std::cout << "Number of elements: ";
	std::cin >> nelements;

	std::vector<uint32_t> numbers;
	numbers.reserve(nelements);
	for (uint32_t i = 0; i < nelements; ++i) {
		uint32_t n;
		std::cin >> n;
		numbers.emplace_back(n);
	}

	uint32_t sum;
	std::cout << "Expected sum " << '\n';
	std::cin >> sum;
	std::cout << std::endl;

	SubsetSum algo(numbers, sum);
	algo.Compute();
	auto result = algo.GetResult();
	for (const auto& set : result) {
		std::cout << "Subset { ";
		for (const auto& x : set) {
			std::cout << x << ' ';
		}
		std::cout << "}\n";
	}
	std::cout << std::endl;
	return 0;
}
