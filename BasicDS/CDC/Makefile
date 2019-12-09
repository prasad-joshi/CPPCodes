CPPFLAGS=-std=c++17 -g -ggdb -O3 -DNDEBUG
INCLUDE=-I. -I/usr/local/include/cryptopp/
LIBS=-lpthread -lcryptopp
CC=clang++

all: main

main: main.cpp
	$(CC) $(CPPFLAGS) $(INCLUDE) main.cpp -o main $(LIBS)


clean:
	rm -f main
