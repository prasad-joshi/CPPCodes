#ifndef __STACK_H__
#define __STACK_H__

#include <vector>

using std::vector;

template <typename T>
class Stack {
private:
	vector<T> vector_;
public:
	void push(const T& t) {
		vector_.emplace_back(t);
	}

	const T pop() {
		const T p = vector_.back();
		vector_.pop_back();
		return p;
	}

	const T& lookupTop() const {
		return vector_.back();
	}

	bool isEmpty() const {
		return vector_.size() == 0;
	};
	
	size_t size() const {
		return vector_.size();
	};
};

#endif
