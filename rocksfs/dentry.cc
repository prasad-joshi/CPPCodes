#include <string>

#include "dentry.h"

using std::string;

void dentry::initialize(string &path, string &name, string &key, int type)
{
	this->path = path;
	this->name = name;
	this->key  = key;
	this->type = type;
}

