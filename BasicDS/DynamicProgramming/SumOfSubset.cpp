#include <iostream>
#include <algorithm>
#include <vector>

class DPTable {
public:	
	DPTable(std::vector<uint32_t> elements, uint32_t expcected) :
			elements_(std::move(elements)), expcected_(expcected) {
		Initialize();
	}

	bool Get(int i, int j) const noexcept {
		if (i < 0 || i >= elements_.size() || j < 0 || j >= expcected_) {
			return false;
		}
		return table_[i][j];
	}

	void Set(int i, int j, bool value) {
		if (i < 0 || i >= elements_.size() || j < 0 || j >= expcected_) {
			return;
		}
		table_[i][j] = value;
	}

	friend std::ostream& operator << (std::ostream& os, DPTable& table) {
		for (const auto& v : table.table_) {
			for (const auto& b : v) {
				std::cout << b << ' ';
			}
			std::cout << std::endl;
		}
		return os;
	}

private:
	void Initialize() {
		for (int i = 0; i < elements_.size(); ++i) {
			std::vector<bool> t(expcected_, false);
			t[0] = true;
			table_.emplace_back(std::move(t));
		}
	}
private:
	std::vector<uint32_t> elements_;
	uint32_t expcected_;
	std::vector<std::vector<bool>> table_;
};

int main(int argc, char* argv[]) {
	int nelements;
	std::cout << "Enter numbr of elements: ";
	std::cin >> nelements;

	std::vector<uint32_t> elements;
	for (int i = 0; i < nelements;  ++i) {
		uint32_t n;
		std::cin >> n;
		elements.emplace_back(n);
	}
	std::sort(elements.begin(), elements.end());

	uint32_t expected;
	std::cout << "Enter expected sum: ";
	std::cin >> expected;

	DPTable table(elements, expected+1);
	for (int i = 0; i < elements.size(); ++i) {
		auto element = elements[i];
		for (int j = 1; j <= expected; ++j) {
			if (element == j ||
					table.Get(i-1, j) || table.Get(i-1, j-element)) {
				table.Set(i, j, true);
			}
		}
	}

	std::cout << "Table " << '\n' << table << std::endl;
	return 0;
}
