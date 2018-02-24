#ifndef __DENTRY_H__
#define __DENTRY_H__

#include <string>
using std::string;

class dentry {
private:
	string path;
	string name;
	string key;
	int    type;
public:
	void initialize(string &path, string &name, string &key, int type);
	string get_path() { return path; }
	string get_name() { return name; }
	string get_key()  { return key;  }
	int    get_type() { return type; }
};

#endif
