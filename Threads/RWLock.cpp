#include <condition_variable>
#include <mutex>
#include <vector>
#include <thread>
#include <iostream>
#include <cassert>

class RWLock {
public:
	void ReadLock() {
		auto locked = ReadLock(true);
		assert(locked);
	}

	bool TryReadLock() {
		return ReadLock(false);
	}

	void WriteLock() {
		auto locked = WriteLock(true);
		assert(locked);
	}

	bool TryWriteLock() {
		return WriteLock(false);
	}

	bool IsLocked() const {
		std::unique_lock<std::mutex> lock(mutex_);
		return not (lock_state_ == kUnlocked);
	}

	void Unlock() {
		std::unique_lock<std::mutex> lock(mutex_);
		switch (lock_state_) {
		case kUnlocked:
			assert(0);
			break;
		case kRead:
			assert(reader_.acquired_ > 0);
			if (not --reader_.acquired_) {
				lock_state_ = kUnlocked;
			}
			break;
		case kWrite:
			assert(writer_.acquired_ and not reader_.acquired_);
			writer_.acquired_ = false;
			lock_state_ = kUnlocked;
			break;
		}

		if (lock_state_ != kUnlocked) {
			return;
		}

		if (writer_.pending_) {
			writer_.cv_.notify_one();
		} else if (reader_.pending_) {
			reader_.cv_.notify_all();
		}
	}

private:
	bool ReadLock(const bool block) {
		std::unique_lock<std::mutex> lock(mutex_);
		switch (lock_state_) {
		case kUnlocked:
			break;
		case kRead:
		case kWrite: {
			while (writer_.pending_ or lock_state_ == kWrite) {
				if (not block) {
					return false;
				}
				++reader_.pending_;
				reader_.cv_.wait(lock);
				--reader_.pending_;
			}
			break;
		}}
		assert(lock_state_ == kUnlocked or lock_state_ == kRead);
		assert(not writer_.acquired_);
		lock_state_ = kRead;
		++reader_.acquired_;
		return true;
	}

	bool WriteLock(const bool block) {
		std::unique_lock<std::mutex> lock(mutex_);
		switch (lock_state_) {
		case kUnlocked:
			break;
		case kRead:
		case kWrite: {
			if (not block) {
				return false;
			}
			++writer_.pending_;
			while (lock_state_ != kUnlocked) {
				writer_.cv_.wait(lock);
			}
			--writer_.pending_;
			break;
		}}
		assert(not writer_.acquired_ and lock_state_ == kUnlocked);
		lock_state_ = kWrite;
		writer_.acquired_ = true;
		return true;
	}

private:
	enum LockType {
		kUnlocked,
		kRead,
		kWrite,
	};

	mutable std::mutex mutex_;
	LockType lock_state_{RWLock::kUnlocked};

	struct {
		std::condition_variable cv_;
		int32_t pending_{};
		int32_t acquired_{};
	} reader_;

	struct {
		std::condition_variable cv_;
		int32_t pending_{};
		bool acquired_{};
	} writer_;
};

struct SomeDS {
	RWLock lock;
	uint64_t i{};
};
SomeDS ds;

void ReaderFunc(int iterations) {
	for (auto i = 0; i < iterations; ++i) {
		ds.lock.ReadLock();
		std::this_thread::yield();
		ds.lock.Unlock();
	}
}

void WriteFunc(int iterations) {
	for (auto i = 0; i < iterations; ++i) {
		ds.lock.WriteLock();
		++ds.i;
		ds.lock.Unlock();
	}
}

int main(int argc, char* argv[]) {
	auto nreaders = std::atoi(argv[1]);
	auto nwriters = std::atoi(argv[2]);
	auto iterations = std::atoi(argv[3]);

	std::vector<std::thread> readers;
	for (auto i = 0; i < nreaders; ++i) {
		readers.emplace_back([&] () mutable {
			ReaderFunc(iterations);
		});
	}

	std::vector<std::thread> writers;
	for (auto i = 0; i < nwriters; ++i) {
		writers.emplace_back([&] () mutable {
			WriteFunc(iterations);
		});
	}

	for (auto& thread : readers) {
		thread.join();
	}
	for (auto& thread : writers) {
		thread.join();
	}
	std::cout << ds.i << ' '
		<< iterations * nwriters
		<< std::endl;
	assert(ds.i == iterations * nwriters);
	return 0;
}
