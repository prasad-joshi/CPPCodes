#pragma once

#include <vector>
#include <optional>

template <typename T>
class Stack {
public:
	Stack(size_t size) : size_(size) {
		elements_.reserve(size_);
	}

	bool Push(const T& element) {
		if (IsFull()) {
			return false;
		}
		elements_.push_back(element);
		return true;
	}

	void Pop() {
		if (IsEmpty()) {
			return;
		}
		elements_.pop_back();
	}

	std::optional<T> Peek() const {
		if (IsEmpty()) {
			return {};
		}
		return elements_.back();
	}

	bool IsEmpty() const {
		return elements_.empty();
	}

	bool IsFull() const {
		return elements_.size() == size_;
	}

	friend std::ostream& operator << (std::ostream& os, const Stack<T>& stack) {
		for (const auto& element : stack.elements_) {
			os << element << " ";
		}
		os << std::endl;
		return os;
	}
private:
	std::vector<T> elements_;
	size_t size_{0};
};
