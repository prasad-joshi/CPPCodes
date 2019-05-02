TARGETS := FindDevice
all: $(TARGETS)

FindDevice: FindDevice.cpp
	clang++ -std=c++17 -g -ggdb FindDevice.cpp -o FindDevice -lpcap

clean:
	rm -f $(TARGETS)
