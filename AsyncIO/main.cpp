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

#if 0
	/* third write --- enabling this nested write results in segment fault */
	IO io3(fd, IO_SIZE, 1024, IOType::WRITE);
	auto f = asyncio.ioSubmit(io3);
	f.then([&io3, &asyncio, &base, &fd] (ssize_t result) {
		assert(result == IO_SIZE);

		IO io4(fd, IO_SIZE, 2048, IOType::WRITE);
		auto f = asyncio.ioSubmit(io4);
		f.then([&] (ssize_t result) {
			assert(result == IO_SIZE);
			base.terminateLoopSoon();
		});
	});
#endif

	base.loopForever();

	close(fd);
}