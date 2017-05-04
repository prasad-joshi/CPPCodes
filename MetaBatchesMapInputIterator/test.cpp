#include <iostream>
#include <map>
#include <vector>

#include "MetaBatchesMapInputIterator.h"

using namespace std;

void fill(map<int, vector<int64_t>>& metaMap) {
	int c = 0;

	for (auto k = 0; k < 3; k++) {
		metaMap[k].clear();
		for (auto v = 0; v < 10; v++) {
			metaMap[k].push_back(++c);
		}
	}
}

int main() {
	map<int, vector<int64_t>> metaMap;

	fill(metaMap);

	MetaBatchesMapInputIterator input(metaMap);

	for (auto it = input.begin(); it != input.end(); it++) {
		cout << *it << " ";
	}
	cout << endl;

	MetaBatchesMapInputIterator input1(metaMap);
	for (auto& e : input1) {
		cout << e << " ";
	}
	cout << endl;
	return 0;
}