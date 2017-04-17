CPPFLAGS := -std=c++14 -O3
LIBS := -lboost_serialization -lgflags

all: main

main: main.cpp Token.cpp parser.cpp
	g++ -o $@  $(CPPFLAGS) $^ $(LIBS)

clean:
	rm -f main
