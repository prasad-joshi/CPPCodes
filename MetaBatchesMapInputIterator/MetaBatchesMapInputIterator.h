#ifndef __META_BATCHES_MAP_INTPUT_ITERATOR_H__
#define __META_BATCHES_MAP_INTPUT_ITERATOR_H__

#include <vector>
#include <map>

class MetaBatchesMapInputIterator {
public:
	typedef struct Iterator {
		Iterator(MetaBatchesMapInputIterator *parentp, bool end) : end_(end),
				parentp_(parentp) {
			if (end_ == false) {
				mapIter_ = parentp->metaMap_.begin();
				if (mapIter_ == parentp_->metaMap_.end()) {
					end_ = true;
				} else {
					vecIter_ = mapIter_->second.begin();
				}
			}
		}

		int operator* () {
			return *vecIter_;
		}

#if 0
		int* operator-> () {
			return vecIter_;
		}
#endif

		Iterator& operator++ () {
			if (end_ == true) {
				return *this;
			}

			vecIter_++;
			if (vecIter_ == mapIter_->second.end()) {
				mapIter_++;
				if (mapIter_ != parentp_->metaMap_.end()) {
					vecIter_ = mapIter_->second.begin();
				} else {
					end_ = true;
				}
			}
		}

		Iterator operator++ (int) {
			auto p = *this;
			++*this;
			return p;
		}

		bool operator== (const Iterator& rhs) {
			return this->end_ == rhs.end_;
		}

		bool operator!= (const Iterator& rhs) {
			return this->end_ != rhs.end_;
		}

	private:
		std::map<int, std::vector<int64_t>>::iterator mapIter_;
		std::vector<int64_t>::iterator vecIter_;
		bool                           end_;
		MetaBatchesMapInputIterator    *parentp_;
	} iterator;
	friend struct Iterator;

	MetaBatchesMapInputIterator(std::map<int, std::vector<int64_t>>& metaMap) :
			metaMap_(metaMap) {

	}

	Iterator begin() {
		return Iterator(this, false);
	}

	Iterator end() {
		return Iterator(this, true);
	}
private:
	std::map<int, std::vector<int64_t>>& metaMap_;
};

#endif