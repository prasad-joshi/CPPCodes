#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <folly/futures/Future.h>
#include <folly/io/async/EventBase.h>
#include <folly/io/async/EventHandler.h>

#include "AsyncIO.h"

using namespace folly;

#define IO_SIZE 4096

int main() {
	AsyncIO   asyncio{32};
	EventBase base;
	asyncio.init(&base);

	int fd = open("/tmp/file.txt", O_RDWR | O_DIRECT | O_CREAT);
	assert(fd > 0);

	/* first write */
	IO io(fd, IO_SIZE, 0, IOType::WRITE);
	char *b = io.getIOBuffer();
	std::memset(b, 'A', IO_SIZE);
	asyncio.ioSubmit(io).then([&io] (ssize_t result) {
		assert(result == IO_SIZE);
	});

	/* second write */
	IO io2(fd, IO_SIZE, 512, IOType::WRITE);
	std::memset(io2.getIOBuffer(), 'B', IO_SIZE);
	asyncio.ioSubmit(io2).then([&io2, &base] (ssize_t result) {
		assert(result == IO_SIZE);
	});

	/* third write */
	IO io3(fd, IO_SIZE, 0, IOType::WRITE);
	std::memset(io3.getIOBuffer(), 'C', IO_SIZE);
	auto f  = asyncio.ioSubmit(io3);
	auto f1 = f.then([&io3] (ssize_t result) {
		assert(result == IO_SIZE);
	});

	IO io4(fd, IO_SIZE, 0, IOType::WRITE);
	std::memset(io4.getIOBuffer(), 'D', IO_SIZE);
	f1.then([&io4, &base, &asyncio] {
		auto f = asyncio.ioSubmit(io4);
		f.then([&io4, &base] (ssize_t result) {
			assert(result == IO_SIZE);
			base.terminateLoopSoon();
		});
	});

	base.loopForever();

	close(fd);
}