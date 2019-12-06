#include <cassert>

namespace dedup {

template <typename Algo>
class Chunker {
public:
	Chunker(
				Algo&& algo
			) :
				algo_(std::forward<Algo>(algo)) {
	}

	template <typename ForwardIt>
	std::pair<ForwardIt, bool> Chunkify(ForwardIt start, ForwardIt end) {
		return algo_.Chunkify(start, end);
	}
private:
	Algo algo_;
};

template <typename Hasher>
class BSW {
public:
	BSW(
				Hasher&& hasher
			):
				hasher_(std::forward<Hasher>(hasher)) {
		hasher_.SetSequenceLength(size_.min_);
	}

	void SetMinChunkSize(size_t size) noexcept {
		assert(IsPowerOfTwo(size));
		size_.min_ = size;
		hasher_.SetSequenceLength(size);
	}

	void SetMaxChunkSize(size_t size) noexcept {
		assert(IsPowerOfTwo(size));
		size_.max_ = size;
	}

	void SetRecommendedChunkSize(size_t size) noexcept {
		assert(IsPowerOfTwo(size));
		size_.mask_ = size - 1;
	}

	template <typename ForwardIt>
	std::pair<ForwardIt, bool> Chunkify(ForwardIt start, ForwardIt end) {
		hasher_.Reset();

		size_t c = 0;
		for (auto it = start; it != end; ++it, ++c) {
			auto v = hasher_.Update(*it);
			if (c <= size_.min_) {
				continue;
			}
			if ((v & size_.mask_) == 0) {
				return {it+1, true};
			}
			if (c >= size_.max_) {
				return {it+1, true};
			}
		}
		return {end, false};
	}

private:
	bool IsPowerOfTwo(size_t size) noexcept {
		return (size & (size - 1)) == 0;
	}
private:
	Hasher hasher_;
	struct {
		size_t min_{512 * kOneKB};
		size_t max_{2 * kOneMB};
		size_t mask_{kOneMB - 1};
	} size_;

private:
	static constexpr size_t kOneKB{1024};
	static constexpr size_t kOneMB{kOneKB * kOneKB};
};

}
