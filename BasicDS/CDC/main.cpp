#define _LARGEFILE64_SOURCE
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstdlib>

#include <cryptopp/sha.h>
#include <cryptopp/filters.h>
#include <cryptopp/hex.h>

#include <unordered_map>
#include <vector>
#include <string>
#include <limits>
#include <thread>
#include <mutex>

#include <iostream>

#include "RollingHash.h"
#include "EncodeDecode.h"
#include "Chunker.h"
#include "BSW.h"
#include "tttd.h"

class File {
public:
	enum class SeekType {
		kCanSeek,
		kCannotSeek
	};

	File(std::string path, SeekType seek = SeekType::kCanSeek) : path_(path) {
		fd_ = open(path.c_str(), O_RDONLY | O_DIRECT);
		assert(fd_ >= 0);
		switch (seek) {
		case SeekType::kCanSeek:
			size_ = Size();
			break;
		case SeekType::kCannotSeek:
			break;
		}
		std::cout << "File " << path_ << " size " << size_ << std::endl;
	}

	~File() {
		if (fd_ != -1) {
			close(fd_);
		}
	}

	size_t Size() {
		auto offset = lseek64(fd_, 0, SEEK_END);
		assert(offset > 0);
		assert(lseek64(fd_, 0, SEEK_SET) >= 0);
		return offset;
	}

	bool Read(char* buf, size_t count, uint64_t offset) {
		if (offset + count >= size_) {
			std::cout << offset + count << ' ' << size_ << std::endl;
			return false;
		}
		ssize_t r = SafeRead(buf, count, offset);
		if (r != count) {
			std::cout << "Path " << path_
				<< " Read " << r
				<< " Expected " << count
				<< std::endl;
		}
		assert(r == count);
		return true;
	}

private:
	ssize_t SafeRead(char* buf, size_t count, uint64_t offset) {
		size_t to_read = count;
		ssize_t copied = 0;
		while (to_read > 0) {
			ssize_t r = pread(fd_, buf, to_read, offset);
			if (r < 0) {
				if (errno == EAGAIN) {
					continue;
				}
				break;
			}
			buf += r;
			to_read -= r;
			offset += r;
			copied += r;
		}
		return copied;
	}
private:
	std::string path_;
	size_t size_{std::numeric_limits<size_t>::max()};
	int fd_{-1};
};

std::vector<uint8_t> Fill(size_t size, char c) {
	std::vector<uint8_t> result(c, size);
	return result;
}

std::string CryptoHash(const unsigned char* buf, const size_t size) {
	CryptoPP::SHA1 hash;
	std::array<CryptoPP::byte, CryptoPP::SHA1::DIGESTSIZE> digest;
	hash.CalculateDigest(digest.data(), (const CryptoPP::byte*)buf, size);

	std::string output;
	CryptoPP::HexEncoder encoder;
	CryptoPP::StringSink test = CryptoPP::StringSink(output);
	encoder.Attach(new CryptoPP::StringSink(output));
	encoder.Put(digest.data(), digest.size());
	encoder.MessageEnd();
	return output;
}

std::string NewChunkReady(const unsigned char* begin, const unsigned char* end) {
	const unsigned char* buf = begin;
	size_t size = end - begin;

	std::string hash = CryptoHash(buf, size);

#if 0
	std::cout << "Chunk Ready"
		<< " Size = " << data.size()
		<< " Hash = " << hash
		<< std::endl;
#endif
	return hash;
}

struct Range {
	uint32_t start;
	uint32_t end;

	Range(uint32_t start, uint32_t end) {
		this->start = start;
		this->end = end;
	}
};

class Histogram {
public:
	Histogram(std::vector<Range> ranges) {
		for (const auto& range : ranges) {
			vec_.emplace_back(range, 0);
		}
	}

	void IncrementRange(uint32_t v) {
		auto it = vec_.rbegin();
		auto eit = vec_.rend();

		bool incremented = false;
		for (; it != eit; ++it) {
			if (v >= it->first.start and v <= it->first.end) {
				++it->second;
				incremented = true;
				break;
			}
		}
		assert(incremented == true);
	}

	friend std::ostream& operator << (std::ostream& os, const Histogram& rhs);
private:
	std::vector<std::pair<Range, uint32_t>> vec_;
};

std::ostream& operator << (std::ostream& os, const Histogram& rhs) {
	for (const auto& v : rhs.vec_) {
		os << "\t{" << v.first.start << ',' << v.first.end << "} = " << v.second << '\n';
	}
	return os;
}

class DedupData {
public:
	DedupData(size_t size) noexcept : size_(size) { }

	size_t Size() const noexcept {
		return size_;
	}

	void ReferenceAdd() noexcept {
		++ref_cnt_;
	}

	uint32_t Reference() const noexcept {
		return ref_cnt_;
	}

private:
	const size_t size_;
	uint32_t ref_cnt_{1};
};

class DedupTable {
public:
	void HashRefer(std::string hash, size_t size) {
		std::lock_guard lock(mutex_);
		auto it = hash_ref_table_.find(hash);
		if (it == hash_ref_table_.end()) {
			hash_ref_table_.emplace(hash, size);
			return;
		}
		assert(it->second.Size() == size);
		it->second.ReferenceAdd();
	}

	void DumpTable() const noexcept {
		std::cout << "\n***** Dumping Dedup Reference Table *****" << std::endl;
		for (const auto& it : hash_ref_table_) {
			std::cout << it.first << ' ' << it.second.Reference() << std::endl;
		}
	}

	Histogram RefCountHistogram() const {
		return Histogram{std::vector<Range>{
			{1, 1},
			{2, 2},
			{3, 3},
			{4, 4},
			{5, 5},
			{6, 6},
			{7, 7},
			{8, 8},
			{9, 9},
			{10, 100},
			{101, 1000},
			{1001, 10000},
			{10001, 100000},
			{100001, 1000000},
			{1000001, 10000000},
		}};
	}

	Histogram SizeHistogram() const {
		return std::vector<Range>{
#if 0
			{0, 4095},
			{4096, 8191},
			{8192, 12287},
			{12288, 16383},
			{16384, 20479},
			{20480, 24575},
			{24576, 28671},
			{28672, 32767},
			{32768, 36863},
#else
			{0, 524287},
			{524288, 1048576},
			{1048577, 1572864},
			{1572865, 2097152},
			{2097153, 2621440},
			{2621441, 3145728},
			{3145729, 3670016},
			{3670017, 4194304},
			{4194305, 4718593},
#endif
		};
	}

	void DumpStats() const noexcept {
		auto hist_ref = RefCountHistogram();
		auto hist_size = SizeHistogram();
		uint32_t min = std::numeric_limits<uint32_t>::max();
		uint32_t max = std::numeric_limits<uint32_t>::min();
		uint32_t non_one = 0;
		uint32_t only_one = 0;
		size_t data_size = 0;
		size_t data_size_after_dedup = 0;

		for (const auto& it : hash_ref_table_) {
			auto ref = it.second.Reference();
			if (ref > 1) {
				non_one++;
			} else {
				only_one++;
			}
			hist_ref.IncrementRange(ref);
			hist_size.IncrementRange(it.second.Size());

			size_t s = it.second.Size();
			data_size += s * ref;
			data_size_after_dedup += s;

			min = std::min(min, ref);
			max = std::max(max, ref);
		}

		std::cout << "===============\n"
			<< "Total Hashes = " << hash_ref_table_.size() << '\n'
			<< "Hash reference min = " << min << '\n'
			<< "Hash reference max = " << max << '\n'
			<< "Hashes with (ref > 1) = " << non_one << '\n'
			<< "Hashes with (ref == 1) = " << only_one << '\n'
			<< "Processed Size (MB) = " << data_size / 1024 / 1024 << '\n'
			<< "Size after dedup (MB) = " << data_size_after_dedup / 1024 / 1024 << '\n'
			<< "Dedup % = " << 100 - (100 * data_size_after_dedup / data_size) << '\n'
			<< "Hash reference Histogram\n" << hist_ref << '\n'
			<< "Chunk size Histogram\n" << hist_size << '\n'
			<< "==============="
			<< std::endl;
	}
private:
	std::mutex mutex_;
	std::unordered_map<std::string, DedupData> hash_ref_table_;
};

void CreateDedupHashes(
			std::string path,
			File::SeekType seek,
			size_t read_begin,
			size_t read_end,
			DedupTable* table,
			bool dump_stats = false
		) {
	RollingHash::KarpRabinHash hasher{64};

	dedup::TTTD algo(std::move(hasher));
	algo.SetMinChunkSize(512 * 1024);
	algo.SetRecommendedChunkSize(1024 * 1024);
	algo.SetMaxChunkSize(4 * 1024 * 1024);

	dedup::Chunker chunker(std::move(algo));

	const size_t kReadSize = 100 * 1024 * 1024;
	unsigned char* data;
	posix_memalign(reinterpret_cast<void**>(&data), 4096, kReadSize);
	if (data == nullptr) {
		return;
	}
	File file(path, seek);

	uint64_t i = 0;
	uint64_t offset = read_begin;
	while (offset < read_end and
			file.Read((char*)data, kReadSize, offset)) {
		auto end = data + kReadSize;

		for (auto it = data; it != end; ) {
			auto [stopped, ready] = chunker.Chunkify(it, end);
			if (ready || stopped == end) {
				auto hash = NewChunkReady(it, stopped);
				table->HashRefer(hash, std::distance(it, stopped));
			}
			it = stopped;
		}
		if (dump_stats and i % 5 == 0) {
			std::cout << "**** " << path << " Iteration " << i << std::endl;
			table->DumpStats();
		}
		++i;
		offset += kReadSize;
	}
	free(data);
	std::cout << "DONE"<< std::endl;
}

int main() {
	const size_t kThreads = 4;
	DedupTable table_sda;
	File sda("/dev/sda", File::SeekType::kCanSeek);
	size_t x = sda.Size() / kThreads;

	std::vector<std::thread> threads;
	for (int i = 0; i < kThreads; ++i) {
		size_t begin = i * x;
		size_t end = (i+1) * x;

		threads.emplace_back(
			[table = &table_sda, stats=(i == 0), begin, end] () mutable {
				CreateDedupHashes("/dev/sda", File::SeekType::kCanSeek, begin, end - 1, table, stats);
			}
		);
	}

	for (auto& thread : threads) {
		thread.join();
	}

	std::cout << "Final Stats " << std::endl;
	table_sda.DumpStats();
	return 0;
}

#if 0

Hello,

I thought I will provide some update on Dedup.

I wrote a PoC code for create dedup hashes. Rather than creating fixed size
chunks (like in STORD where block size is fixed 16K), the code is based on
Content Defined Chunking (CDC).

CDC is based on finger print (or Rolling Hash). For experiment, code uses
Rabin Karp 64 bit finger print. It creates a rolling hash for every byte we
process (read from disk). Chunk is created once a finger print has certain
property. There are many CDC algorithms, I am going to experiment with

1. Basic Sliding Window
2. TTTD

1. Basic Sliding Window
=====
It is based on 3 parameters
a. Minimum Chunk Size
b. Max Chunk Size
c. Recommended Chunk Size

The code uses following values
Min = 4K
Recommended = 16K
Max = 32K

It cuts a chunk when
     (Rabin Karp Finger print & (Recommended - 1)) == 0 OR
	 (Number of bytes processed) >= Max-Chunk-Size

Once the chunk boundary is defined. The code computes SHA1 hash of the chunk,
adds in the map with reference count. The code incrementes the dedup reference,
if hash is found in the map.

I did not precondition my hard disk for this run. I did not create any files on
disk which are suitable for Dedup.

Here is output after processing 150GB of my hard disk ('/dev/sda')

===============
Total Hashes = 7685148
Hash reference min = 1
Hash reference max = 381143
Hashes with (ref > 1) = 18357
Hashes with (ref == 1) = 7657888
Processed Size (MB) = 155877
Size after dedup (MB) = 149459
Dedup % = 5
Hash reference Histogram
	{1,1} = 7657888
	{2,2} = 16612
	{3,3} = 1232
	{4,4} = 263
	{5,5} = 87
	{6,6} = 21
	{7,7} = 18
	{8,8} = 12
	{9,9} = 7
	{10,100} = 97
	{101,1000} = 5
	{1001,10000} = 2
	{10001,100000} = 0
	{100001,1000000} = 1
	{1000001,10000000} = 0
===============

Total Hashes -> Total hashes seen
Size saved after dedup = 155877 - 149459 = 6418 (MB)
Dedup % = 5

Hashes with (ref > 1) = 18357
  ==> 18357 Chunks are dedupped
Hashes with (ref == 1) = 7657888
  ==> 7657888 chunks are not dedupped

Hash reference max = 381143
  ==> Probably a block with all zeros
  ==> The same block is seen 381143 times

Hash histogram dumps some more usefull information
{1,1} = 7657888 ==> number of chunks which are referenced once (non deduped chunks)
{100001,1000000} = 1 ==> again probably chunk with all zero's
{2,2} = 16612 ==> 16612 chunks were seen exactly twice
{3,3} = 1232 ==> 1232 chunks were seen exactly thrice





Final Stats after all data is processed
===============
Total Hashes = 14980080
Hash reference min = 1
Hash reference max = 842474
Hashes with (ref > 1) = 51799
Hashes with (ref == 1) = 14928281
Processed Size (MB) = 305900
Size after dedup (MB) = 291317
Dedup % = 5
Hash reference Histogram
	{1,1} = 14928281
	{2,2} = 46522
	{3,3} = 2750
	{4,4} = 1712
	{5,5} = 415
	{6,6} = 181
	{7,7} = 71
	{8,8} = 10
	{9,9} = 14
	{10,100} = 101
	{101,1000} = 18
	{1001,10000} = 4
	{10001,100000} = 0
	{100001,1000000} = 1
	{1000001,10000000} = 0

===============



Final Stats
===============
Total Hashes = 14841874
Hash reference min = 1
Hash reference max = 802788
Hashes with (ref > 1) = 179985
Hashes with (ref == 1) = 14661889
Processed Size (MB) = 305900
Size after dedup (MB) = 288615
Dedup % = 6
Hash reference Histogram
	{1,1} = 14661889
	{2,2} = 140666
	{3,3} = 26879
	{4,4} = 9358
	{5,5} = 2327
	{6,6} = 570
	{7,7} = 45
	{8,8} = 13
	{9,9} = 3
	{10,100} = 103
	{101,1000} = 19
	{1001,10000} = 1
	{10001,100000} = 0
	{100001,1000000} = 1
	{1000001,10000000} = 0

===============

#endif
