#include <vector>
#include <functional>
#include <utility>
#include <iostream>

template <typename T, typename Comp = std::greater_equal<T>>
class PriorityQueue {
public:
	PriorityQueue() {
		/* first element is unused */
		queue_.emplace_back(0);
	}

	template <typename ForwardIt>
	PriorityQueue(ForwardIt it, ForwardIt eit) {
		queue_.emplace_back(0);
		std::copy(it, eit, std::back_inserter(queue_));
		Heapify();
	}

	const T& Top() const noexcept {
		return queue_[1];
	}

	bool IsEmpty() const noexcept {
		return queue_.size() == 1;
	}

	void Push(T element) {
		queue_.emplace_back(element);
		ShiftUp(queue_.size() - 1);
	}

	void Pop() {
		queue_[1] = std::move(queue_.back());
		queue_.pop_back();
		ShiftDown(1);
	}

private:
	void Heapify() {
		auto mid = (queue_.size() - 1) / 2;
		for (auto i = mid; i >= 1; --i) {
			ShiftDown(i);
		}
	}

	void ShiftDown(size_t index) {
		auto element = std::move(queue_[index]);
		auto size = queue_.size() - 1;
		auto child = index * 2;
		for (; child <= size ;) {
			if (child < size and func(queue_[child+1], queue_[child])) {
				++child;
			}
			if (not func(queue_[child], element)) {
				break;
			}
			queue_[index] = std::move(queue_[child]);
			index = child;
			child *= 2;
		}
		queue_[index] = std::move(element);
	}

	void ShiftUp(size_t index) {
		auto element = std::move(queue_[index]);
		for (; index > 1 and not func(queue_[index/2], element); index /= 2) {
			queue_[index] = std::move(queue_[index/2]);
		}
		queue_[index] = std::move(element);
	}
private:
	std::vector<T> queue_;
	Comp func{};
};

template <typename T>
using MaxHeap = PriorityQueue<T>;

template <typename T>
using MinHeap = PriorityQueue<T, std::less_equal<T>>;

int main(int argc, char* argv[]) {
	std::vector<int> elements;
	for (int i = 1; argv[i]; ++i) {
		auto n = std::atoi(argv[i]);
		elements.push_back(n);
	}

	{
		MaxHeap<int> max;
		for (const auto& e : elements) {
			max.Push(e);
		}

		std::cout << "Max heap ";
		while (not max.IsEmpty()) {
			std::cout << max.Top() << ' ';
			max.Pop();
		}
		std::cout << '\n';
	}

	{
		MinHeap<int> min;
		for (const auto& e : elements) {
			min.Push(e);
		}

		std::cout << "Min heap ";
		while (not min.IsEmpty()) {
			std::cout << min.Top() << ' ';
			min.Pop();
		}
		std::cout << '\n';
	}

	{
		MaxHeap<int> max(elements.begin(), elements.end());
		std::cout << "Max heap ";
		while (not max.IsEmpty()) {
			std::cout << max.Top() << ' ';
			max.Pop();
		}
		std::cout << '\n';
	}

	return 0;
}
