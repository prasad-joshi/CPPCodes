#ifndef __LINK_LIST_H__
#define __LINK_LIST_H__

template<typename T>
struct node {
	const T data;
	node<T> *nextp;

	node() : node(0) {

	}

	node(const T& value) : data(value), nextp(nullptr) {
	}
};

template<typename T>
class linklist {
private:
	struct node<T> head;
public:

	typedef struct ConstIterator {
	private:
		linklist ownerp;
		node<T>  *curp;

	public:
		ConstIterator(const linklist& owner, node<T> *nodep) :
			ownerp(owner), curp(nodep) {
		}

		const T& operator* () const {
			return curp->data;
		}

		const T* operator& () const {
			return &curp->data;
		}

		const ConstIterator& operator++ () {
			curp = curp->nextp;
			return *this;
		}

		ConstIterator operator++ (int /* unused */) {
			auto p = *this;
			++*this;
			return p;
		}

		bool operator== (const ConstIterator& rhs) {
			return this->curp == rhs.curp;
		}

		bool operator!= (const ConstIterator& rhs) {
			return this->curp != rhs.curp;
		}
	} const_iterator;
	
	friend ConstIterator;

	linklist() {
		head.nextp = nullptr;
	}

	void add(const T& data) {
		auto np    = new node<T>(data);
		np->nextp  = head.nextp;
		head.nextp = np;
	}

	const_iterator cbegin() {
		return ConstIterator(*this, head.nextp);
	}

	const_iterator cend() {
		return ConstIterator(*this, nullptr);
	}
};

#endif