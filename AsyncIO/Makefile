INC := -I.
LIBS := -levent -laio -lpthread -lfolly -lgflags -lglog
CPPCLAGS := -g -ggdb -O0

SOURCES := AsyncIO.cpp main.cpp
OBJECTS := $(SOURCES:.cpp=.o)

all: main

main: $(OBJECTS)
	g++ -std=c++14 $(CPPCLAGS) $(INC) -o $@ $^ $(LIBS)

%.o : %.cpp
	g++ -std=c++14 $(CPPCLAGS) $(INC) -c $^

clean:
	rm -rf main *.o
