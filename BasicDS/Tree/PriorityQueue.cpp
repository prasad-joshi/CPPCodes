#include <vector>
#include <functional>
#include <utility>
#include <iostream>

template <typename T, typename Comp = std::less_equal<T>>
class PriorityQueue {
public:
	PriorityQueue() {
		/* first element is unused */
		queue_.emplace_back(0);
	}

	const T& Top() const noexcept {
		return queue_[1];
	}

	bool IsEmpty() const noexcept {
		return queue_.size() == 1;
	}

	void Push(T element) {
		/* increase the size */
		queue_.emplace_back(0);

		auto index = queue_.size() - 1;
		for (; index > 1 and func(queue_[index/2], element); index /= 2) {
			queue_[index] = std::move(queue_[index/2]);
		}
		queue_[index] = std::move(element);
	}

	void Pop() {
		auto removed = std::move(queue_[1]);
		auto last = std::move(queue_.back());
		queue_.pop_back();

		size_t size = queue_.size() - 1;
		size_t current_node = 1;
		size_t child = 2;
		while (child <= size) {
			if (child < size and func(queue_[child], queue_[child+1])) {
				++child;
			}

			if (not func(last, queue_[child])) {
				break;
			}

			queue_[current_node] = std::move(queue_[child]);
			current_node = child;
			child *= 2;
		}
		queue_[current_node] = std::move(last);
	}
private:
	std::vector<T> queue_;
	Comp func{};
};

template <typename T>
using MaxHeap = PriorityQueue<T>;

template <typename T>
using MinHeap = PriorityQueue<T, std::greater_equal<T>>;

int main(int argc, char* argv[]) {
	{
		MaxHeap<int> max;
		for (int i = 1; argv[i]; ++i) {
			auto n = std::atoi(argv[i]);
			max.Push(n);
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
		for (int i = 1; argv[i]; ++i) {
			auto n = std::atoi(argv[i]);
			min.Push(n);
		}

		std::cout << "Min heap ";
		while (not min.IsEmpty()) {
			std::cout << min.Top() << ' ';
			min.Pop();
		}
		std::cout << '\n';
	}

	return 0;
}
