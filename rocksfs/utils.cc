#include <string>
#include <algorithm>
#include <vector>
#include <sstream>

#include <gcrypt.h>
#include "utils.h"

using std::string;
using std::vector;

static bool both_slash_chars(char l, char r)
{
	return l == r && l == '/';
}

/*
 * Convert to correct path
 * */
void path_convert(string &path)
{
	/* remove unnecessary '/' characters from path */
	string::iterator end = std::unique(path.begin(), path.end(),
			both_slash_chars);
	path.erase(end, path.end());

	if (path.length() != 0 && path.at(0) != '/') {
		/*
		 * path does not start with '/' -- add '/' at the start
		 * */
		path.insert(0, "/");
	}

	/* make path start with '/root' */
	string p = "/root";
	path.insert(0, p);

	if (path.at(path.length() - 1) == '/') {
		/* remove last slash if any */
		path.pop_back();
	}
}

static void split(const string &path, char delim, vector<string> &elems)
{
	std::stringstream ss;
	std::string       i;

	ss.str(path);
	while (std::getline(ss, i, delim)) {
		elems.push_back(i);
	}
}

vector<string> path_components(string &path)
{
	vector<string> elems;

	path_convert(path);
	split(path, '/', elems);
	return elems;
}

void sha1(string &in, string &out)
{
	auto l  = in.length();
	int  hl = gcry_md_get_algo_dlen(GCRY_MD_SHA1);
	char hash[hl];

	gcry_md_hash_buffer(GCRY_MD_SHA1, hash, in.c_str(), l);

	for (int i = 0; i < hl; i++) {
		out += std::to_string(hash[i]);
	}
}
