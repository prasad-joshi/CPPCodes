#pragma once

namespace dedup {

template <typename Hasher>
class FixSizeChunking {
public:
	FixSizeChunking(Hasher&& unused) {}
	FixSizeChunking() {}

	void SetMinChunkSize(size_t unused) noexcept {}

	void SetMaxChunkSize(size_t unused) noexcept {}

	void SetRecommendedChunkSize(size_t size) noexcept {
		size_ = size;
	}

	template <typename ForwardIt>
	std::pair<ForwardIt, bool> Chunkify(ForwardIt start, ForwardIt end) {
		size_t s = std::distance(start, end);
		return s < size_ ?
			std::pair{end, false} :
			std::pair{std::next(start, s), true};
	}

private:
	size_t size_;
};

}
