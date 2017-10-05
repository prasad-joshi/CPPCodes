/*
PALIN - The Next Palindrome

A positive integer is called a palindrome if its representation in the decimal
system is the same when read from left to right and from right to left. For a
given positive integer K of not more than 1000000 digits, write the value of the
smallest palindrome larger than K to output. Numbers are always displayed
without leading zeros.

Input

The first line contains integer t, the number of test cases. Integers K are
given in the next t lines.

Output

For each K, output the smallest palindrome larger than K.

Example

Input:
2
808
2133

Output:
818
2222

*/

#include <iostream>
#include <string>
#include <cassert>

bool contains_same(const std::string& str, const char ch = '9') {
	for (const auto& c : str) {
		if (c != ch) {
			return false;
		}
	}
	return true;
}

std::string strip_leading_zeros(std::string& str) {
	auto bit = str.begin();
	auto eit = str.end();

	for (; bit != eit; ++bit) {
		if (*bit != '0') {
			break;
		}
	}
	str.erase(str.begin(), bit);
	return str;
}

char next_char(char ch) {
	if (ch == '9') {
		return '0';
	}
	return ch+1;
}

size_t set_end(char* datap, size_t e, const char end_char) {
	assert(e >= 0);
	auto cur = datap[e];
	datap[e] = end_char;

	if (cur < end_char) {
		return e;
	}

	--e;
	while (e >= 0) {
		auto c   = next_char(datap[e]);
		datap[e] = c;
		if (c != '0' || e == 0) {
			break;
		}
		--e;
	}
	return e;
}

std::string next_palindrome(std::string result) {
	if (result.front() == '0') {
		strip_leading_zeros(result);
		if (result.empty()) {
			/* string contains all zeros */
			return std::string("1");
		}
	}

	auto all_9 = contains_same(result, '9');
	if (all_9) {
		/* string contain all 9's */
		size_t length = result.length();
		auto r = std::string(length + 1, '0');
		r.front() = '1';
		r.back() = '1';
		return r;
	}

	/*
	 * code below assumes
	 * - string does not contain all 9's
	 */

	size_t length = result.length();
	if (length == 1) {
		auto c = result.front();
		result.back() = next_char(c);
		return result;
	}

	/*
	 * code below assumes
	 * - string does not contain all 9's
	 * - string.length() > 1
	 */
	std::string::size_type start  = 0;
	std::string::size_type end    = length - 1;
	auto                   datap  =  const_cast<char *>(result.c_str());

	/* by default increment string by one */
	set_end(datap, end, *(datap + start));

	while (start < end) { // TODO: ensure condition is correct
		if (datap[start] == datap[end]) {
			++start;
			--end;
			continue;
		}

		auto modified = set_end(datap, end, datap[start]);
		assert(modified >= start);
		if (modified == start) {
			continue;
		}
		++start;
		--end;
	}
	return result;
}

int main() {
	int64_t ntests;
	std::cin >> ntests;
	while (ntests--) {
		std::string line;
		std::cin >> line;

		auto result = next_palindrome(std::move(line));
		std::cout << result << std::endl;
	}
}