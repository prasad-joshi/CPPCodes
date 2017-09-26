#include <iostream>
#include "linklist.h"

using std::cout;
using std::endl;

int main() {
	node<int>     a(10);
	linklist<int> list;

	list.add(10);
	list.add(20);
	list.add(30);
	list.add(40);
	list.add(50);
	list.add(60);
	list.add(70);

	for (auto it = list.cbegin(); it != list.cend(); it++) {
		cout << *it << endl;
	}
	return 0;
}