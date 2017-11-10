all:
	g++ -c Calculator.cpp -o Calculator.o -std=c++14
	gcc -c main.c -o main.o -I.
	g++ main.o Calculator.o -o example

clean:
	rm *.o example