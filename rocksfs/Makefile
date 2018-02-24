CPPFLAGS := -Wall -Werror -ggdb -O0 -std=c++11  -DROCKSDB_PLATFORM_POSIX -DROCKSDB_LIB_IO_POSIX  -DOS_LINUX -fno-builtin-memcmp -DROCKSDB_FALLOCATE_PRESENT -DSNAPPY -DZLIB -DBZIP2 -DNUMA -DROCKSDB_MALLOC_USABLE_SIZE -DROCKSDB_PTHREAD_ADAPTIVE_MUTEX -DROCKSDB_BACKTRACE -march=native

LDFLAGS := -lrocksdb -lrt -lsnappy -lz -lbz2 -lnuma -lpthread -lgcrypt

INC := -Ilib/json/src/ -I.

all: rocksfs

rocksfs: rocksfs.cc utils.cc dentry.cc rocksfile.cc base64.cc
	g++ $(INC) $(CPPFLAGS) -o $@ $^ $(LDFLAGS)

clean:
	rm -f rocksfs
