#include <vector>
#include <rocksdb/db.h>

#include "json.hpp"
#include "rocksfile.h"
#include "base64.h"

using namespace rocksdb;
using std::shared_ptr;
using std::string;
using std::vector;
using json = nlohmann::json;

inline bool rocksfile_impl::is_block_aligned(off_t offset)
{
	return (offset & bs_mask) == 0;
}

uint32_t rocksfile_impl::blocks_count(off_t offset, size_t length)
{
	bool     a;
	uint32_t c;

	c = 0;
	a = is_block_aligned(offset);
	if (a == false) {
		size_t l = bs - (offset - bs_mask);
		if (length < l) {
			return 1;
		}
		c       = 1;
		length -= l;
	}

	c += (length >> bs_shift);
	if (length & bs_mask) {
		c++;
	}
	return c;
}

void rocksfile_impl::io_blocks(off_t offset, size_t count, uint32_t &start,
		uint32_t &end)
{
	start = offset >> bs_shift;
	end   = (offset + count - 1) >> bs_shift;
}

rocksfile_impl::rocksfile_impl(string &key, uint32_t block_size)
{
	/* block size must be power of 2 */
	assert(block_size && ((block_size & (block_size - 1)) == 0));

	this->key     = key;
	this->refcnt  = 0;
	this->deleted = false;
	this->bs      = block_size;
	this->bs_mask = block_size - 1;

	bs_shift = 0;
	while (bs_mask & (1ull << bs_shift)) {
		bs_shift++;
	}
}

uint32_t rocksfile_impl::open()
{
	return ++refcnt;
}

uint32_t rocksfile_impl::close(bool &unlink)
{
	uint32_t rc = --refcnt;
	unlink = false;
	if (rc == 0 && deleted == true) {
		unlink = true;
	}
	return rc;
}

void rocksfile_impl::encode(json &j, uint32_t block, char *buf,
		size_t sz, uint64_t offset)
{
	assert(sz == bs);

	j[F_BLOCK]  = block;
	j[F_DATA]   = base64_encode((BYTE *) buf, sz);
	j[F_SIZE]   = sz;
	j[F_OFFSET] = offset;
}

void rocksfile_impl::decode(json &j, uint32_t block, char *buf,
		size_t sz, uint64_t offset)
{
	assert(sz == bs);

	uint32_t b = j[F_BLOCK];
	uint32_t s = j[F_SIZE];
	uint64_t o = j[F_OFFSET];

	assert(offset == o && b == block && s == sz);
	string eb = j[F_DATA];
	vector<BYTE> dv = base64_decode(eb);
	assert(dv.size() <= sz);
	std::copy(dv.begin(), dv.end(), buf);
}

#if 0
void rocksfile_impl::read_modify(rocksdb::DB *dbp, string &k, json &j,
		uint32_t block, char *buf, size_t size, off_t offset)
{
	assert(0);
	string v;
	auto   s = dbp->Get(ReadOptions(), k, &v);
	if (s.ok()) {
		auto     rj     = json::parse(v);
		uint32_t rblock = rj[F_BLOCK];

		assert(block == rblock);
	} else {
		encode(j, block, buf, size, offset);
	}
}
#endif

ssize_t rocksfile_impl::write(rocksdb::DB *dbp, char *buf, size_t count,
			off_t offset)
{
	uint32_t sb;
	uint32_t eb;

	/* find blocks to be written */
	io_blocks(offset, count, sb, eb);

	/* TODO: write lock the range */

	/* divide into blocks and write data */
	rocksdb::WriteBatch batch;
	char   *b     = buf;
	size_t copied = 0;
	size_t bc     = 0;
	while (sb <= eb) {
		json   j;
		string k = key + ":" + std::to_string(sb);

		bool a = is_block_aligned(offset);
		if (a == false) {
			bc = bs - (bs_mask & offset);
		} else {
			bc = bs;
		}

		if (bc > count) {
			bc = count;
		}

		if (a == false) {
			/* read-modify-write */
			assert(0);
			a  = true;
			// read_modify(dbp, j, sb, b, bc, offset);
		} else {
			encode(j, sb, b, bc, offset);
		}

		batch.Put(k, j.dump());

		copied += bc;
		offset += bc;
		b      += bc;
		count  -= bc;
		sb++;
	}

	auto s = dbp->Write(WriteOptions(), &batch);

	/* TODO: unlock lock */

	if (s.ok()) {
		return copied;
	}

	return -ENOSPC;
}

ssize_t rocksfile_impl::read(rocksdb::DB *dbp, char *buf, size_t count,
			off_t offset)
{
	uint32_t sb;
	uint32_t eb;
	string   v;

	/* find blocks to be written */
	io_blocks(offset, count, sb, eb);

	/* TODO: read lock the range */
	/* divide into blocks and write data */
	char   *b     = buf;
	size_t copied = 0;
	size_t bc     = 0;

	while (sb <= eb) {
		string k = key + ":" + std::to_string(sb);

		bool a = is_block_aligned(offset);
		if (a == false) {
			bc = bs - (bs_mask & offset);
		} else {
			bc = bs;
		}

		if (bc > count) {
			bc = count;
		}

		auto s = dbp->Get(ReadOptions(), k, &v);
		if (!s.ok()) {
			break;
		}
		auto j = json::parse(v);

		if (a == false) {
			/* partial read */
			assert(0);
			a  = true;
		} else {
			decode(j, sb, b, bc, offset);
		}

		copied += bc;
		offset += bc;
		b      += bc;
		count  -= bc;
		sb++;
	}

	/* TODO: unlock lock */

	return copied;
}

rocksfile::rocksfile(shared_ptr<rocksfile_impl> fi, string &key, int fd)
{
	this->fi   = fi;
	this->fd   = fd;
	this->key  = key;
}

int rocksfile::open(DB *dbp, int fd)
{
	assert(this->fd == fd);
	fi->open();
	return 0;
}

int rocksfile::close(DB *dbp, int fd, bool &release, bool &unlink)
{
	assert(this->fd == fd);
	uint32_t c = fi->close(unlink);
	if (c == 0) {
		release = true;
	} else {
		release = false;
	}
	return 0;
}

ssize_t rocksfile::pwrite(rocksdb::DB *ddb, void *buf, size_t count,
			off_t offset)
{
	return fi->write(ddb, (char *) buf, count, offset);
}

ssize_t rocksfile::pread(rocksdb::DB *ddb, void *buf, size_t count,
			off_t offset)
{
	return fi->read(ddb, (char *) buf, count, offset);
}

