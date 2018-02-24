# rocksfs - Simple File System on rocksdb

The intention is to learn rocksdb APIs. Here is the problem statement.

* Build a rudimentary file system on top of rocksdb.
* No need to support POSIX compliant APIs. Rather support the following set of simple APIs.
	* CreateDir(dirpath)
	* RemoveDir(dirpath)
	* CreateFile(FullyQualifiedFileName)
	* RemoveFile(FullyQualifiedFileName)
	* WriteFile(FullyQualifiedFileName, fileoffset, buflen, pbuf)
	* ReadFile(FullyQualifiedFileName, fileoffset, buflen, pbuf)
	* ListDir(dirpath)
	* fstat(FullyQualifiedFileName, &statInfo)
	* truncate(FullyQualifiedFileName, newSize)

## HOW TO COMPILE

After cloning run following commands

```
$ git submodule init

$ git submodule update

$ make
```
