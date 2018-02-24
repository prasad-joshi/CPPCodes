#ifndef __ROCKSFILE_H__
#define __ROCKSFILE_H__

#include <rocksdb/db.h>
#include <stdint.h>

#include "json.hpp"
using json = nlohmann::json;

const std::string F_BLOCK  = "/FB";
const std::string F_DATA   = "/FD";
const std::string F_SIZE   = "/FS";
const std::string F_OFFSET = "/FO";

class rocksfile_impl {
private:
	std::string key;
	uint32_t    refcnt;
	bool        deleted;
	uint32_t    bs;
	uint32_t    bs_mask;
	uint32_t    bs_shift;

private:
	bool     is_block_aligned(off_t offset);
	uint32_t blocks_count(off_t off, size_t count);
	void     io_blocks(off_t offset, size_t count,
			uint32_t &start, uint32_t &end);
	void encode(json &j, uint32_t block, char *buf, size_t sz,
			uint64_t offset);
	void decode(json &j, uint32_t block, char *buf, size_t sz,
			uint64_t offset);
public:
	rocksfile_impl(std::string &key, uint32_t block_size);
	uint32_t open();
	uint32_t close(bool &unlink);
	void mark_deleted() { this->deleted = true; }

	ssize_t write(rocksdb::DB *ddb, char *buf, size_t count,
			off_t offset);
	ssize_t read(rocksdb::DB *ddb, char *buf, size_t count,
			off_t offset);
};

class rocksfile {
private:
	std::shared_ptr<rocksfile_impl> fi;
	std::string key;
	int         fd;
public:
	rocksfile(std::shared_ptr<rocksfile_impl> fi, std::string &key, int fd);
	int open(rocksdb::DB *dbp, int fd);
	int close(rocksdb::DB *dbp, int fd, bool &release, bool &unlink);
	std::string get_key() { return this->key; }

	ssize_t pwrite(rocksdb::DB *ddb, void *buf, size_t count,
			off_t offset);
	ssize_t pread(rocksdb::DB *ddb, void *buf, size_t count,
			off_t offset);
};

#endif
