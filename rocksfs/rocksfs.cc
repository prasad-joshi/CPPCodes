#include <errno.h>
#include <string>
#include <sstream>
#include <ios>

#include <gcrypt.h>
#include <time.h>
#include <rocksdb/db.h>
#include <rocksdb/options.h>

#include "json.hpp"
#include "utils.h"
#include "dentry.h"
#include "rocksfile.h"

#define SEC_TO_NSEC(s) ((s) * 1000ull * 1000ull * 1000ull)
#define DEFAULT_BLOCK_SIZE (4096)

const string KW_NAME  = "/name";
const string KW_PARENT= "/parent";
const string KW_KEY   = "/key";
const string KW_TYPE  = "/type";
const string KW_BS    = "/bs";
const string KW_BSA   = "/bsa";
const string KW_FS    = "/fs";
const string KW_ATIME = "/atime";
const string KW_CTIME = "/ctime";
const string KW_MTIME = "/mtime";
const string KW_DEL   = "/deleted";

using namespace rocksdb;
using std::string;
using std::vector;
using std::ios_base;
using std::shared_ptr;
using json = nlohmann::json;

class rocksfs {
private:
	/* do not create copies of rocksfs object */
	void operator=(rocksfs const &fs) = delete;
	rocksfs(rocksfs const &) = delete;

private:
	const string DATADB_PATH = "/tmp/data.db";
	const string ROOT_KEY    = "root";

	/* few keywords */
	static const vector<string> keywords; /* for searching */

	enum {
		FS_FILE,
		FS_DIRECTORY,
	};

	DB *ddbp; /* data DB connector */

private: /* file handling DS */

	std::map<string, shared_ptr<rocksfile_impl>> open_file_table;
	std::map<int, shared_ptr<rocksfile>> fd_table;

private:
	bool is_keyword(string &n);
	bool is_dir_empty(string dir_key);
	void calc_key(string &path, string &key);
	void dir_json_init(json &j, string &name, string &key,
			string &parent);
	void file_json_init(json &j, string &name, string &key,
			string &parent);

	void fd_allocate(int &fd);
	void fd_deallocate(int fd);
	void fd_set(int fd, shared_ptr<rocksfile> f);
	shared_ptr<rocksfile> fd_search(int fd);

	shared_ptr<rocksfile_impl> mark_file_open(string &key, uint32_t bs);
	void mark_file_closed(string &key);
	void mark_file_deleted(string &key);
	bool is_file_open(string &key);

private:
	int mkfs();
	int namei(string &path, dentry &dp);
	int dir_lookup(json &dirjson, string &name, json &entry);
	int lookup(string &path, string &name, dentry &parent);
	void lazy_unlink(string &key);

public:
	rocksfs();
	~rocksfs();

	int mkdir(string dir);
	int list_dir(string path);
	int rmdir(string dir, bool recursive = false);

	int create(string file);
	int unlink(string file);
	int open(string file, int &fd);
	int close(int fd);
	ssize_t pwrite(int fd, void *buf, size_t count, off_t offset);
	ssize_t pread(int fd, void *buf, size_t count, off_t offset);
};

const vector<string> rocksfs::keywords = [] {
	vector<string> v;
	v.push_back(KW_NAME);
	v.push_back(KW_PARENT);
	v.push_back(KW_KEY);
	v.push_back(KW_TYPE);
	v.push_back(KW_BS);
	v.push_back(KW_BSA);
	v.push_back(KW_FS);
	v.push_back(KW_ATIME);
	v.push_back(KW_CTIME);
	v.push_back(KW_MTIME);
	v.push_back(KW_DEL);
	return v;
}();

/* few basic private functions */
bool rocksfs::is_keyword(string &n)
{
	return std::find(keywords.begin(), keywords.end(), n) != keywords.end();
}

bool rocksfs::is_dir_empty(string dir_key)
{
	string v;

	auto s = ddbp->Get(ReadOptions(), dir_key, &v);
	auto j = json::parse(v);

	for (json::iterator it = j.begin(); it != j.end(); it++) {
		auto n = it.key();
		if (is_keyword(n) == false) {
			return false;
		}
	}
	return true;
}

void rocksfs::calc_key(string &path, string &key)
{
	/* combine path + timestamp - create SHA1 hash */
	struct timespec ts;
	auto rc = clock_gettime(CLOCK_MONOTONIC, &ts);
	assert(rc == 0);

	string t = path + ":" +
		std::to_string(SEC_TO_NSEC(ts.tv_sec) + ts.tv_nsec);

	sha1(t, key);
}

void rocksfs::dir_json_init(json &j, string &name, string &key, string &parent)
{
	j[KW_NAME]   = name;
	j[KW_KEY]    = key;
	j[KW_PARENT] = parent;
	j[KW_TYPE]   = FS_DIRECTORY;
	j[KW_DEL]    = 0;
}

void rocksfs::file_json_init(json &j, string &name, string &key, string &parent)
{
	j[KW_NAME]   = name;
	j[KW_KEY]    = key;
	j[KW_PARENT] = parent;
	j[KW_TYPE]   = FS_FILE;
	j[KW_DEL]    = 0;
	j[KW_BS]     = DEFAULT_BLOCK_SIZE;
	j[KW_BSA]    = 0;
	j[KW_FS]     = 0;
	j[KW_ATIME]  = 0;
	j[KW_CTIME]  = 0;
	j[KW_MTIME]  = 0;
}

void rocksfs::fd_allocate(int &fd)
{
	std::map<int, shared_ptr<rocksfile>>::iterator it;
	fd = 0;
	for (it = fd_table.begin(); it != fd_table.end(); it++) {
		if (it->first == fd) {
			fd++;
			continue;
		} else {
			return;
		}
	}
}

void rocksfs::fd_deallocate(int fd)
{
	fd_table[fd] = nullptr;
	fd_table.erase(fd);
}

void rocksfs::fd_set(int fd, shared_ptr<rocksfile> f)
{
	auto it = fd_table.find(fd);
	assert(it == fd_table.end());
	fd_table[fd] = f;
}

shared_ptr<rocksfile> rocksfs::fd_search(int fd)
{
	auto it = fd_table.find(fd);
	if (it != fd_table.end()) {
		return it->second;
	}
	return nullptr;
}

shared_ptr<rocksfile_impl> rocksfs::mark_file_open(string &key, uint32_t bs)
{
	shared_ptr<rocksfile_impl> fi;
	auto it = open_file_table.find(key);
	if (it == open_file_table.end()) {
		/* entry not found */
		fi = std::make_shared<rocksfile_impl>(key, bs);
		open_file_table[key] = fi;
	} else {
		fi = it->second;
	}
	return fi;
}

void rocksfs::mark_file_closed(string &key)
{
	open_file_table[key] = nullptr;
	open_file_table.erase(key);
}

bool rocksfs::is_file_open(string &key)
{
	return open_file_table.find(key) != open_file_table.end();
}

void rocksfs::mark_file_deleted(string &key)
{
	auto it = open_file_table.find(key);
	assert(it != open_file_table.end());
	shared_ptr<rocksfile_impl> fi = it->second;
	fi->mark_deleted();
}

rocksfs::rocksfs()
{
	Options options;
	string  rootv;

	options.IncreaseParallelism();
	options.OptimizeLevelStyleCompaction();
	options.create_if_missing = true;

	ddbp   = NULL;
	auto s = DB::Open(options, DATADB_PATH, &ddbp);
	assert(s.ok() && ddbp != NULL);

	s = ddbp->Get(ReadOptions(), ROOT_KEY, &rootv);
	if (s.IsNotFound()) {
		/* root directory not found - create file system */
		auto rc = mkfs();
		if (rc < 0) {
			goto error;
		}
	}

	s = ddbp->Get(ReadOptions(), ROOT_KEY, &rootv);
	assert(s.ok());

	return;
error:
	if (ddbp != NULL) {
		delete ddbp;
		ddbp = NULL;
	}
}

rocksfs::~rocksfs()
{
	if (ddbp != NULL) {
		delete ddbp;
		ddbp = NULL;
	}
}

int rocksfs::mkfs()
{
	json   dr;
	string root   = "root"; /* name of the root directory */
	string parent = "root"; /* key of the parent directory */
	string key    = "root"; /* key used to access this directory */

	dir_json_init(dr, root, parent, key);

	assert(ddbp != NULL);
	auto s = ddbp->Put(rocksdb::WriteOptions(), ROOT_KEY, dr.dump());
	if (!s.ok()) {
		return -1;
	}

	return 0;
}

/*
 * Given directory JSON from data DB, check for directory entry inside 
 * the directory.
 *
 * dirjson - is directory JSON from data DB
 * name - name to search
 * entry - found entry
 *
 * returns 0 on success
 * */
int rocksfs::dir_lookup(json &dirjson, string &name, json &entry)
{
	int del = dirjson[KW_DEL];
	if (del != 0) {
		return -ENOENT;
	}
	auto it = dirjson.find(name);
	if (it != dirjson.end()) {
		entry = it.value();
		return 0;
	}
	return -ENOENT;
}

int rocksfs::namei(string &path, dentry &dp)
{
	std::stringstream ss(path);
	string c;
	Status s;

	string mvj; /* meta DB value in serialized JSON */
	string dvj; /* data DB value in serialized JSON */
	json   mdirj;
	json   ddirj;
	bool   root_seen;

	root_seen = false;
	while (std::getline(ss, c, '/')) {
		if (c.empty()) {
			continue;
		}
//		std::cout << c << " eof " << ss.eof() << std::endl;

		if (root_seen == false) {
			if (c != string("root")) {
				return -ENOTDIR;
			}

			s = ddbp->Get(ReadOptions(), c, &dvj);
			assert(s.ok());
			ddirj = json::parse(dvj);
			int type = ddirj[KW_TYPE];
			assert(type == FS_DIRECTORY &&
					ddirj[KW_NAME] == string("root"));
			root_seen = true;
			if (ss.eof()) {
				dp.initialize(path, c, c, FS_DIRECTORY);
				break;
			}
			continue;
		}

		assert(root_seen == true);

		/* search name is part of parent directory */
		json entry;
		auto rc = dir_lookup(ddirj, c, entry);
		if (rc < 0) {
			return rc;
		}

		string key  = entry[KW_KEY];
		int    type = entry[KW_TYPE];
		if (ss.eof() == true) {
			/* all path components have been processed */
			dp.initialize(path, c, key, type);
			break;
		} else if (type != FS_DIRECTORY) {
			/* file must always be last component of path */
			return -ENOTDIR;
		}

		/* read data DB entry for current path component */
		s = ddbp->Get(ReadOptions(), key, &dvj);
		assert(s.ok());
		ddirj = json::parse(dvj);
	}
	// std::cout << ss.eof() << std::endl;
	return 0;
}

int rocksfs::lookup(string &path, string &name, dentry &parent)
{
	auto f   = path.find_last_of('/');
	auto dir = path.substr(0, f);
	auto rc  = namei(dir, parent);

	if (rc < 0) {
		return rc;
	}

	if (parent.get_type() == FS_DIRECTORY) {
		name = path.substr(f+1);
		return 0;
	}
	return -ENOTDIR;
}

int rocksfs::mkdir(string dir)
{
	int    rc;
	dentry de;

	path_convert(dir);
	std::cout << "DIR : " << dir << std::endl;
	rc = namei(dir, de);
	if (rc == 0) {
		return -EEXIST;
	}

	dentry pde;
	string name;
	rc = lookup(dir, name, pde);
	if (rc < 0) {
		return rc;
	}

	/* generate key for directory */
	string dk;
	calc_key(dir, dk);

	/* create JSON for directory */
	json j;
	string pdk = pde.get_key();
	dir_json_init(j, name, dk, pdk);

	/* read parent directory JSON and add a new directory entry into it */
	string v;
	auto   s = ddbp->Get(ReadOptions(), pdk, &v);
	assert(s.ok());
	json pdirj = json::parse(v);
	pdirj[name] = {{KW_KEY, dk}, {KW_TYPE, FS_DIRECTORY}};

	rocksdb::WriteBatch batch;
	batch.Put(pdk, pdirj.dump());
	batch.Put(dk, j.dump());
	s = ddbp->Write(WriteOptions(), &batch);
	if (s.ok()) {
		return 0;
	}
	return -EINVAL;
}


int rocksfs::rmdir(string dir, bool recursive)
{
	assert(recursive == false);

	if (dir.length() == 0) {
		return -ENOTDIR;
	}
	path_convert(dir);

	dentry de;
	auto   rc = namei(dir, de);
	if (rc < 0) {
		return rc;
	}

	if (de.get_type() != FS_DIRECTORY) {
		/* not a directory */
		return -ENOTDIR;
	}

	if (is_dir_empty(de.get_key()) ==  false) {
		return -ENOTEMPTY;
	}

	string name;
	dentry pde;
	rc = lookup(dir, name, pde);
	if (rc < 0) {
		return rc;
	}

	/* read parent directory's JSON */
	string v;
	auto s = ddbp->Get(ReadOptions(), pde.get_key(), &v);
	assert(s.ok());

	auto j  = json::parse(v);
	auto it = j.find(name);
	assert(it != j.end());
	j.erase(it); /* remove dir's entry from JSON */

	/* write new JSON and delete directory's key */
	rocksdb::WriteBatch batch;
	batch.Put(pde.get_key(), j.dump());
	batch.Delete(de.get_key());
	s = ddbp->Write(WriteOptions(), &batch);
	if (s.ok()) {
		return 0;
	}
	return -EINVAL;
}

int rocksfs::list_dir(string path)
{
	dentry de;

	path_convert(path);

	auto rc = namei(path, de);
	if (rc < 0) {
		return rc;
	}

	string v;
	auto s = ddbp->Get(ReadOptions(), de.get_key(), &v);
	assert(s.ok());

	auto j = json::parse(v);
	for (json::iterator it = j.begin(); it != j.end(); it++) {
		auto n = it.key();
		if (is_keyword(n) == true) {
			continue;
		}
		auto j = it.value();
		std::cout << it.key() << " " << j[KW_TYPE] << std::endl;
	}
	std::cout << std::endl;
	return 0;
}

int rocksfs::create(string file)
{
	path_convert(file);

	dentry de;
	auto   rc = namei(file, de);
	if (rc == 0) {
		return -EEXIST;
	}

	dentry pde;
	string name;
	rc = lookup(file, name, pde);
	if (rc < 0) {
		return rc;
	}

	string fk;
	calc_key(file, fk);

	json j;
	string dk = pde.get_key();
	file_json_init(j, name, dk, fk);

	string v;
	auto   s = ddbp->Get(ReadOptions(), dk, &v);
	assert(s.ok());
	json pdj  = json::parse(v);
	pdj[name] = {{KW_KEY, fk}, {KW_TYPE, FS_FILE}};

	rocksdb::WriteBatch batch;
	batch.Put(dk, pdj.dump());
	batch.Put(fk, j.dump());
	s = ddbp->Write(WriteOptions(), &batch);
	if (s.ok()) {
		return 0;
	}
	return -EINVAL;
}

void rocksfs::lazy_unlink(string &key)
{
	string v;
	auto s  = ddbp->Get(ReadOptions(), key, &v);
	assert(s.ok());

	auto fj = json::parse(v);
	int del = fj[KW_DEL];
	assert(del == 1);

	rocksdb::WriteBatch batch;
	/* TODO: additionally delete all file data blocks */
	batch.Delete(key);
	s = ddbp->Write(WriteOptions(), &batch);
	assert(s.ok());
}

int rocksfs::unlink(string file)
{
	dentry de;

	path_convert(file);
	auto rc = namei(file, de);
	if (rc < 0) {
		return rc;
	}

	if (de.get_type() != FS_FILE) {
		return -EISDIR;
	}

	string name;
	dentry pde;
	rc = lookup(file, name, pde);
	if (rc < 0) {
		return rc;
	}

	json   fj; /* file json */
	string fk   = de.get_key();
	bool   open = is_file_open(fk);
	if (open == true) {
		/* file is open - mark file for deletion */
		/* 
		 * if file is open - we only update the parent's directory
		 * entry. The file's key and DB entry is kept around until
		 * last instance of file is closed.
		 * */
		string v;
		auto s = ddbp->Get(ReadOptions(), fk, &v);
		fj = json::parse(v);
		fj[KW_DEL] = 1;
		mark_file_deleted(fk);
	}

	string v;
	auto s = ddbp->Get(ReadOptions(), pde.get_key(), &v);
	assert(s.ok());

	auto pdj = json::parse(v);
	auto it  = pdj.find(name);
	assert(it != pdj.end());
	pdj.erase(it); /* remove dir's entry from JSON */

	/* write new JSON and delete directory's key */
	rocksdb::WriteBatch batch;
	batch.Put(pde.get_key(), pdj.dump());
	if (open == false) {
		/* TODO: additionally delete all file data blocks */
		batch.Delete(de.get_key());
	} else {
		batch.Put(de.get_key(), fj.dump());
	}

	s = ddbp->Write(WriteOptions(), &batch);
	if (s.ok()) {
		return 0;
	}
	return -EINVAL;
}

int rocksfs::open(string file, int &fd)
{
	dentry de;

	path_convert(file);
	auto rc = namei(file, de);
	if (rc < 0) {
		return rc;
	}

	if (de.get_type() != FS_FILE) {
		return -EISDIR;
	}

	/* search if file is already opened */
	string fk = de.get_key();
	string v;
	auto   s  = ddbp->Get(ReadOptions(), fk, &v);
	if (!s.ok()) {
		return -ENOENT;
	}

	auto     j  = json::parse(v);
	uint32_t bs = j[KW_BS];

	shared_ptr<rocksfile_impl> fi;
	fi = mark_file_open(fk, bs);
	assert(fi);

	fd_allocate(fd);
	assert(fd >= 0);
	// std::cout << "FD = " << fd << std::endl;

	auto f = std::make_shared<rocksfile>(fi, fk, fd);
	assert(f);
	fd_set(fd, f);

	f->open(ddbp, fd);

	return 0;
}

int rocksfs::close(int fd)
{
	auto f = fd_search(fd);
	if (f == nullptr) {
		return -EBADF;
	}

	bool release;
	bool unlink;
	f->close(ddbp, fd, release, unlink);

	if (release == true) {
		string key = f->get_key();
		mark_file_closed(key);
		if (unlink == true) {
			lazy_unlink(key);
		}
	}

	f = nullptr;
	fd_deallocate(fd);
	return 0;
}

ssize_t rocksfs::pwrite(int fd, void *buf, size_t count, off_t offset)
{
	auto f = fd_search(fd);
	if (f == nullptr) {
		return -EBADF;
	}

	auto rc = f->pwrite(ddbp, buf, count, offset);

	return rc;
}

ssize_t rocksfs::pread(int fd, void *buf, size_t count, off_t offset)
{
	auto f = fd_search(fd);
	if (f == nullptr) {
		return -EBADF;
	}

	auto rc = f->pread(ddbp, buf, count, offset);

	return rc;
}

void test_dir(rocksfs &fs)
{
	auto rc = fs.mkdir("/a");
	std::cout << "dir /a creation returned: " << rc << std::endl;

	rc = fs.mkdir("/b");
	std::cout << "dir /b creation returned: " << rc << std::endl;

	rc = fs.mkdir("/b/b/");
	std::cout << "dir /b/b creation returned: " << rc << std::endl;

	std::cout << "===========" << std::endl;
	std::cout << "default dir listing " << std::endl;
	rc = fs.list_dir("");
	assert(rc == 0);

	std::cout << "dir /a listing " << std::endl;
	rc = fs.list_dir("/a");
	assert(rc == 0);

	std::cout << "dir /b listing " << std::endl;
	rc = fs.list_dir("/b");
	assert(rc == 0);

	std::cout << "dir /b/b listing " << std::endl;
	rc = fs.list_dir("/b/b");
	assert(rc == 0);

	std::cout << "dir /c listing " << std::endl;
	rc = fs.list_dir("/c");
	assert(rc != 0);

	rc = fs.rmdir("");
	assert(rc == -ENOTDIR);
	rc = fs.rmdir("/c");
	assert(rc != 0);
	rc = fs.rmdir("/b");
	assert(rc == -ENOTEMPTY);
	rc = fs.rmdir("/b/b");
	assert(rc == 0);
	rc = fs.rmdir("/b");
	assert(rc == 0);
	rc = fs.rmdir("/a");
	assert(rc == 0);
}

void t_file1(rocksfs &fs)
{
	auto rc = fs.create("a.txt");
	assert(rc == 0);

	rc = fs.create("a.txt");
	assert(rc != 0);

	for (int i = 0; i < 10; i++) {
		int fd1, fd2;

		rc = fs.open("a.txt", fd1);
		assert(fd1 == 0 && rc == 0);

		rc = fs.open("a.txt", fd2);
		assert(fd2 == 1 && rc == 0);

		fs.close(fd1);
		fs.close(fd2);
	}

	rc = fs.unlink("a.txt");
	assert(rc == 0);

}

void t_file2(rocksfs &fs)
{
	auto rc = fs.create("a.txt");
	assert(rc == 0);

	int fd1;
	rc = fs.open("a.txt", fd1);
	assert(fd1 == 0 && rc == 0);

	fs.unlink("a.txt");
	assert(rc == 0);

	int fd2;
	rc = fs.open("a.txt", fd2);
	assert(rc != 0);

	rc = fs.create("a.txt");
	assert(rc == 0);
	for (int i = 0; i < 10; i++) {
		rc = fs.open("a.txt", fd2);
		assert(fd2 == 1 && rc == 0);
		fs.close(fd2);
	}

	fs.close(fd1);

	for (int i = 0; i < 10; i++) {
		rc = fs.open("a.txt", fd2);
		assert(fd2 == 0 && rc == 0);
		fs.close(fd2);
	}

	rc = fs.unlink("a.txt");
	assert(rc == 0);
}

void t_file3(rocksfs &fs)
{
	auto rc = fs.create("a.txt");
	assert(rc == 0);

	int fd;
	rc = fs.open("a.txt", fd);
	assert(fd == 0 && rc == 0);

	char wb[DEFAULT_BLOCK_SIZE];
	std::memset(wb, 'a', sizeof(wb));
	auto bc = fs.pwrite(fd, wb, sizeof(wb), 0);
	assert(bc == sizeof(wb));

	char rb[DEFAULT_BLOCK_SIZE];
	bc = fs.pread(fd, rb, sizeof(rb), 0);
	rc = std::memcmp(wb, rb, sizeof(wb));
	assert(rc == 0);

	fs.close(fd);

	rc = fs.unlink("a.txt");
	assert(rc == 0);
}

void test_file(rocksfs &fs)
{
	t_file1(fs);
	t_file2(fs);
	t_file3(fs);
}

int main(int argc, char *argv[])
{
	rocksfs fs;
	test_dir(fs);
	test_file(fs);
	return 0;
}
