#include <iostream>
#include <iomanip>
#include <vector>
#include <algorithm>
#include <limits>
#include <cassert>

struct Object {
	Object(int32_t w, int32_t v) : weight(w), value(v) {

	}
	int32_t weight;
	int32_t value;

	friend std::ostream& operator << (std::ostream& os, const Object& obj) {
		os << obj.weight << "," << obj.value;
		return os;
	}
};

class KnapsackTable {
public:
	KnapsackTable(std::vector<Object> objects, int32_t weight) :
			objects_(std::move(objects)) {
		InitializeTable(weight);
	}

	size_t ObjectsCount() const noexcept {
		return objects_.size();
	}

	const Object& GetObject(int index) const {
		return objects_[index];
	}

	int32_t Get(ssize_t i, ssize_t j) const noexcept {
		if (j < 0) {
			return std::numeric_limits<int32_t>::min();
		} else if (i < 0) {
			return 0;
		}
		return table_[i][j];
	}

	void Set(ssize_t i, ssize_t j, int32_t value) {
		table_[i][j] = value;
	}

	friend std::ostream& operator << (std::ostream& os, const KnapsackTable& table) {
		table.DumpTable(os);
		return os;
	}

private:
	void DumpTable(std::ostream& os) const {
		auto it = objects_.begin();
		for (const auto& d : table_) {
			std::cout << *it++ << ": ";
			for (const auto v : d) {
				std::cout << std::setw(4) << v << " ";
			}
			std::cout << "\n";
		}
		std::cout << std::endl;
	}

	void InitializeTable(int32_t weight) {
		for (auto i=0u; i < objects_.size(); ++i) {
			std::vector<int32_t> value(weight+1, 0);
			table_.emplace_back(std::move(value));
		}
	}
private:
	std::vector<std::vector<int32_t>> table_;
	std::vector<Object> objects_;
};

KnapsackTable BuildTable(const std::vector<Object>& objects, const int32_t weight) {
	KnapsackTable table(objects, weight);

	for (int i = 0; i < objects.size(); ++i) {
		const auto& object = objects[i];
		for (int j = 1; j <= weight; ++j) {
			auto v1 = table.Get(i - 1, j);
			auto v2 = table.Get(i - 1, j - object.weight) + object.value;

			table.Set(i, j, std::max(v1, v2));
		}
	}
	std::cout << table << std::endl;
	return table;
}

std::vector<Object> GetObjects(const KnapsackTable& table,
		int32_t weight) {
	std::vector<Object> result;
	auto obj_index = table.ObjectsCount() - 1;
	auto value = table.Get(obj_index, weight);
	if (value <= 0) {
		return result;
	}

	while (weight and value) {
		if (value == table.Get(obj_index -1, weight)) {
			--obj_index;
			continue;
		}

		const auto& obj = table.GetObject(obj_index);
		result.emplace_back(obj);
		assert(weight >= obj.weight);
		weight -= obj.weight;
		value -= obj.value;
	}

	std::sort(result.begin(), result.end(),
			[] (const Object& b1, const Object& b2) {
		return b1.weight < b2.weight;
	});
	return result;
}

int main() {
	int nobjects;

	std::vector<Object> objects;
	std::cout << "Enter number of objects ";
	std::cin >> nobjects;
	std::cout << "Enter Object's weight and value ";
	for (auto i = nobjects; i > 0; --i) {
		int32_t w, v;
		std::cin >> w >> v;
		objects.emplace_back(w, v);
	}
	std::sort(objects.begin(), objects.end(),
			[] (const Object& b1, const Object& b2) {
		return b1.weight < b2.weight;
	});

	std::cout << "Enter Knapsack weight limit ";
	int32_t weight;
	std::cin >> weight;
	std::cout << std::endl;


	std::cout << "Objects ";
	for (const auto& d : objects) {
		std::cout << d << " ";
	}
	std::cout << "\n";
	std::cout << "weight limit " << weight << std::endl;

	auto table = BuildTable(objects, weight);
	auto result = GetObjects(table, weight);
	if (result.empty()) {
		std::cout << "Cannot";
	} else {
		int32_t value = 0;
		std::cout << "Objects: ";
		for (const auto& c : result) {
			std::cout << c << " ";
			value += c.value;
		}
		std::cout << "\nMaximum value " << value << std::endl;
	}
	std::cout << std::endl;
	return 0;
}
