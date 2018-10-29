#include <iostream>
#include <chrono>
#include <thread>

#include <cerrno>
#include <cassert>

using namespace std::chrono_literals;

class List {
public:
	struct node {
		node() {
		
		}
		node(int num) : number(num) {
		
		}

		void free() {
			if (nextp) {
				nextp->free();
			}

			delete this;
		}

		int number{0};
		struct node* nextp{nullptr};
	};

public:
	class ListIterator {
	public:
		ListIterator(List& list, List::node* nodep) : list_(list), nodep_(nodep) {
		
		}

		ListIterator& operator++() {
			nodep_ = nodep_->nextp;
			return *this;
		}

		bool operator ==(const ListIterator& rhs) {
			return nodep_ == rhs.nodep_;
		}

		bool operator !=(const ListIterator& rhs) {
			return nodep_ != rhs.nodep_;
		}

		const List::node& operator *() const {
			return *nodep_; 
		}

		List::node& operator *() {
			return *nodep_;
		}

		const List::node* operator ->() const {
			return nodep_;
		}

		List::node* operator ->() {
			return nodep_;
		}

	private:
		List& list_;
		List::node* nodep_{nullptr};
	};

public:
	List() {	
	}

	~List() {
		tailp = nullptr;
		head.nextp->free();
		head.nextp = nullptr;
	}

	node* NewNode(int no) {
		node* nodep = new node(no);
		if (not nodep) {
			throw std::bad_alloc();
		}
		return nodep;
	}

	void InsertFirstNode(node* nodep) {
		head.nextp = nodep;
		tailp = nodep;
	}

	void PushFront(int no) {
		auto nodep = NewNode(no);

		if (head.nextp) {
			nodep->nextp = head.nextp;
			head.nextp = nodep;
			return;
		}

		InsertFirstNode(nodep);
	}

	void PushBack(int no) {
		auto nodep = NewNode(no);
		if (tailp) {
			tailp->nextp = nodep;
			tailp = nodep;
			return;
		}
		InsertFirstNode(nodep);
	}

	ListIterator begin() {
		return ListIterator(*this, head.nextp);
	}

	ListIterator end() {
		return ListIterator(*this, nullptr);
	}

	using iterator = ListIterator;
public:
	struct node head;
	struct node* tailp{nullptr};
};

void Usage(std::string pro) {

}

List::iterator MeetingPoint(List& list) {
	auto sit = list.begin();
	auto fit = list.begin();
	auto eit = list.end();

	for (; fit != eit; ) {
		++fit;
		if (fit == eit) {
			break;
		}

		++fit;
		if (fit == eit) {
			break;
		}

		++sit;
		if (sit == fit) {
			break;
		}
	}
	return fit;
}

bool IsCircular(List& list) {
	return not (MeetingPoint(list) == list.end());
}

void MakeListCircular(List& list, int nelements) {
	auto now = std::chrono::system_clock::now();
	auto epoch = std::chrono::duration_cast<std::chrono::seconds>(
			now.time_since_epoch()).count();
	auto link_to = epoch % nelements;

	auto it = list.begin();
	auto eit = list.end();

	for (; it != eit; ++it) {
		if (it->number == link_to) {
			std::cout << "Found " << it->number << std::endl;
			list.tailp->nextp = &*it;
			break;
		}
	}
}

void FixList(List& list) {
	auto meeting_it = MeetingPoint(list);
	if (meeting_it == list.end()) {
		return;
	}

	auto it = list.begin();
	if (it == meeting_it) {
		list.tailp->nextp = nullptr;
	} else {
		List::node* prev_node = nullptr;
		while (it != meeting_it) {
			prev_node = &*meeting_it;
			++meeting_it;
			++it;
		}
		prev_node->nextp = nullptr;
	}

}

int main(int argc, char* argv[]) {
	if (argc != 2) {
		Usage(argv[0]);
		return EINVAL;
	}

	int nelements = std::atoi(argv[1]);
	if (not nelements) {
		Usage(argv[0]);
		return EINVAL;
	}

	List list;
	for (auto i = 0; i < nelements; ++i) {
		list.PushBack(i);
	}

	for (const auto& nodep : list) {
		std::cout << nodep.number << " ";
	}
	std::cout << std::endl;

	auto circular = IsCircular(list);
	assert(not circular);

	MakeListCircular(list, nelements);

	circular = IsCircular(list);
	assert(circular);

	FixList(list);

	circular = IsCircular(list);
	assert(not circular);

	for (const auto& nodep : list) {
		std::cout << nodep.number << " ";
	}
	std::cout << std::endl;
	return 0;
}
