CPPFLAGS := -std=c++14 -g -ggdb -O0
LIBS := -lboost_serialization

all: main

main: main.cpp Token.cpp parser.cpp
	g++ -o $@  $(CPPFLAGS) $^ $(LIBS)

clean:
	rm -f main
