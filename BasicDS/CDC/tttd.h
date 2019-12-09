#pragma once

#include <iterator>

#if 0
	d1 = 540
	d2 = 270
	MinChunk = 460
	MaxChunk = 2800

	Assuming m is average expected chunk size,
	- multiply each of the above parameter by m/1015
#endif

#define unlikely(expr) (expr)
#define likely(expr) (expr)

namespace dedup {

template <typename Hasher>
class TTTD {
public:
	TTTD(Hasher hasher) noexcept : hasher_(std::move(hasher)) {
		SetRecommendedChunkSize(16*1024);
	}

	void SetMinChunkSize(size_t size) noexcept {
		chunk_size_.min_ = size;
	}

	void SetMaxChunkSize(size_t size) noexcept {
		chunk_size_.max_ = size;
	}

	void SetRecommendedChunkSize(size_t size) noexcept {
		size_t m = size / 1015;
		chunk_size_.main_divisor_ = 540 * m;
		chunk_size_.backup_divisor_ = 270 * m;
	}

	template <typename ForwardIt>
	std::pair<ForwardIt, bool> Chunkify(ForwardIt begin, ForwardIt end) {
		hasher_.Reset();

		ForwardIt backup_it{end};
		size_t consumed = 0;
		for (auto it = begin; it != end; ++it) {
			const auto hash = hasher_.Update(*it);
			if (++consumed < chunk_size_.min_) {
				continue;
			}
			if (hash % chunk_size_.backup_divisor_ == 0) {
				backup_it = it;
			}
			const auto masked = hash % chunk_size_.main_divisor_;
			if (unlikely(masked == 0)) {
				return {it+1, true};
			}
			if (likely(consumed < chunk_size_.max_)) {
				continue;
			}

			if (backup_it != end) {
				return {backup_it+1, true};
			}
			return {it+1, false};
		}
		return {end, false};
	}

	void DumpStats(std::ostream& os) {
		os << "Total Size " << stats_.total_size_ << ' '
			<< "Total chunks " << stats_.nchunks_ << ' '
			<< "Average chunk size " << stats_.total_size_ / stats_.nchunks_ << ' '
			<< "Main chunks " << stats_.nmain_ << ' '
			<< "Max threshold chunks " << stats_.nmax_threshold_ << ' '
			<< "Backup chunks " << stats_.nbackup_ << ' '
			<< "Last small " << stats_.last_small_ << ' '
			<< std::endl;
	}

private:
	Hasher hasher_;
	struct {
		size_t min_{8*1024};
		size_t max_{32*1024};
		size_t main_divisor_{};
		size_t backup_divisor_{};
	} chunk_size_;

	struct {
		uint64_t nbackup_{0};
		uint64_t nmain_{0};
		uint64_t nmax_threshold_{0};
		uint64_t last_small_{0};

		uint64_t nchunks_{0};
		uint64_t total_size_{0};
	} stats_;
};

}
