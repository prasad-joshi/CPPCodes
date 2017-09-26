#include <algorithm>
#include <iostream>
#include <sstream>
#include <cstdint>

class NumRange {
public:
	class NumRangeIterator {
	public:
		NumRangeIterator(int64_t current = 0) : current_(current) {

		}

		int64_t operator * () const {
			return current_;
		}

		NumRangeIterator& operator ++ () {
			++current_;
			return *this;
		}

		bool operator == (const NumRangeIterator& rhs) const {
			return current_ == rhs.current_;
		}

		bool operator != (const NumRangeIterator& rhs) const {
			return not (*this == rhs);
		}

	private:
		int64_t current_;
	};

public:
	using iterator = NumRangeIterator;

	NumRange(int64_t from, int64_t to) : from_(from), to_(to) {

	}

	iterator begin() const {
		return NumRangeIterator{from_};
	}

	iterator end() const {
		return NumRangeIterator{to_};
	}

private:
	int64_t from_;
	int64_t to_;
};

int main() {
	NumRange r{100, 110};

	{
		/* Iterate over numbers */
		std::stringstream os;
		for (const auto& e : r) {
			os << std::to_string(e) << " ";
		}
		os << std::endl;
		std::cout << "Numbers " << os.str();
	}
	{
		std::stringstream os;
		auto it = r.begin();
		auto eit = r.end();

		for (; it != eit; ++it) {
			os << std::to_string(*it) << " ";
		}
		os << std::endl;
		std::cout << "Numbers " << os.str();
	}

	auto p = std::minmax_element(r.begin(), r.end());
	std::cout << "Min " << *p.first << ", Max " << *p.second << std::endl;
	return 0;
}