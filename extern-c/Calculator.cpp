#include <iostream>

class Calculator {
public:
	int Add(int a, int b) {
		return a + b;
	}
};

extern "C"  {
int add(int no1, int no2) {
	Calculator c;
	return c.Add(no1, no2);
}
}
