#pragma once

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

}
