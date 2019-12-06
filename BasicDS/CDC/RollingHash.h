#include <array>
#include <queue>
#include <algorithm>
#include <random>
#include <limits>
#include <cassert>

namespace RollingHash {
template <typename HashValueType>
HashValueType ComputeMask(int bits) {
	assert(bits >= 0);
	assert(sizeof(HashValueType) * 8 >= bits);
	auto x = static_cast<HashValueType>(1) << (bits - 1);
	return x ^ (x - 1);
}

template <typename HashValueType, typename CharType>
class CharacterHash {
public:
	CharacterHash(const int seed, const HashValueType max) {
		InitializeHash(seed, max);
	}

	HashValueType Get(CharType index) const noexcept {
		return hash_values_[index];
	}

private:
	void InitializeHash(const int seed, const HashValueType& max) {
		if constexpr (sizeof(HashValueType) <= 4) {
			InitializeHash_4(seed, max);
		} else if constexpr (sizeof(HashValueType) <= 8) {
			InitializeHash_8(seed, max);
		} else {
			throw std::runtime_error("Unsupported hash value type");
		}
	}

	void InitializeHash_4(const int seed, const HashValueType& max) {
		std::mt19937 gen(seed);
		std::uniform_int_distribution<HashValueType> dist(0, max);
		std::generate(hash_values_.begin(), hash_values_.end(), [&] () {
			return dist(gen);
		});
	}

	void InitializeHash_8(const int seed, const HashValueType& max) {
		std::mt19937 gen(seed);
		std::uniform_int_distribution<HashValueType> dist_bottom(0,
			(max >> 32) == 0 ? max : std::numeric_limits<HashValueType>::max());
		std::uniform_int_distribution<HashValueType> dist_top(0, max >> 32);
		std::generate(hash_values_.begin(), hash_values_.end(), [&] () {
			return dist_top(gen) | dist_bottom(gen);
		});
	}

private:
	std::array<HashValueType, 1 << (sizeof(CharType) * 8)> hash_values_;
};

template <typename HashValueType = uint64_t, typename CharType = unsigned char>
class KarpRabinHash {
public:
	KarpRabinHash(
					int word_size = 64
				):
					hash_mask_(ComputeMask<HashValueType>(word_size)),
					hasher_(7, hash_mask_) {
		for (int i = 0; i < seq_len_; ++i) {
			b_to_n_ *= kB_;
			b_to_n_ &= hash_mask_;
		}
	}

	void SetSequenceLength(uint64_t seq_len) noexcept {
		b_to_n_ = 1;
		seq_len_ = seq_len;
		for (int i = 0; i < seq_len_; ++i) {
			b_to_n_ *= kB_;
			b_to_n_ &= hash_mask_;
		}
	}

	void Reset() {
		hash_value_ = 0;
		while (not seen_.empty()) {
			seen_.pop();
		}
	}

	HashValueType Update(const CharType in) {
		if (seen_.size() < seq_len_) {
			Eat(in);
			return hash_value_;
		}
		CharType out = seen_.front();
		seen_.pop();
		seen_.push(in);

		hash_value_ = (kB_ * hash_value_ + hasher_.Get(in) -
				b_to_n_ * hasher_.Get(out)
			) & hash_mask_;
		return hash_value_;
	}

	HashValueType GetHashValue() const noexcept {
		return hash_value_;
	}

private:
	void Eat(const CharType c) {
		hash_value_ = (kB_ * hash_value_ + hasher_.Get(c)) & hash_mask_;
	}

private:
	uint64_t seq_len_{64};
	const HashValueType hash_mask_;
	const CharacterHash<HashValueType, CharType> hasher_;
	std::queue<CharType> seen_;
	HashValueType b_to_n_{1};
	HashValueType hash_value_{0};
private:
	const HashValueType kB_{37};
};
}
