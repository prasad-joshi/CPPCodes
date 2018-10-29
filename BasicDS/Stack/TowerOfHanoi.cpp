#include <iostream>
#include <algorithm>

#include "Stack.h"

class Disk {
public:
	Disk(int size) : size_(size) {
	
	}

	bool operator > (const Disk& rhs) {
		return size_ > rhs.size_;
	}

	bool operator < (const Disk& rhs) {
		return size_ < rhs.size_;
	}

	friend std::ostream& operator << (std::ostream& os, const Disk& d) {
		os << "Disk" << d.size_;
		return os;
	}
private:
	size_t size_{0};
};

class Tower {
public:
	Tower(int index, int ndisks) : disks_(ndisks), index_(index) {
	
	}

	void PushDisk(Disk d) {
		auto top = disks_.Peek();
		if (not top or top.value() > d) {
			disks_.Push(d);
		} else {
			throw std::bad_alloc();
		}
	}

	friend void MoveDisk(Tower& src, Tower& dst) {
		dst.PushDisk(src.disks_.Peek().value());
		src.disks_.Pop();

	}

	friend std::ostream& operator << (std::ostream& os, Tower& tower) {
		os << "T" << tower.index_ << " " << tower.disks_;
		return os;
	}
private:
	Stack<Disk> disks_;
	int index_{0};
};

void TowerOfHannoi(int moves, Tower& t1, Tower& t2, Tower& t3) {
	if (moves >= 1) {
		TowerOfHannoi(moves-1, t1, t3, t2);
		MoveDisk(t1, t2);
		std::cout << t1 << t2 << std::endl;
		TowerOfHannoi(moves-1, t3, t2, t1);
	}
}

std::vector<int> GenerateNumbers(const int count) {
	std::vector<int> v(count);
	std::generate(v.begin(), v.end(), [c = count] () mutable {
		return c--;
	});
	return v;
}

int main(int argc, char* argv[]) {
	const int kDisks = 3;
	Tower t1(1, kDisks);
	Tower t2(2, kDisks);
	Tower t3(3, kDisks);

	for (auto n : GenerateNumbers(kDisks)) {
		t1.PushDisk(Disk(n));
	}

	TowerOfHannoi(kDisks, t1, t3, t2);
	return 0;
}
