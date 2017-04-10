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
	auto io1 = make_unique<IO>(fd, IO_SIZE, 0, IOType::WRITE);
	char *b = io1->getIOBuffer();
	std::memset(b, 'A', IO_SIZE);
	asyncio.ioSubmit(std::move(io1)).then([ ] (unique_ptr<IO> io1) {
		assert(io1->getResult() == IO_SIZE);
	});

	/* second write */
	auto io2 = make_unique<IO>(fd, IO_SIZE, 512, IOType::WRITE);
	std::memset(io2->getIOBuffer(), 'B', IO_SIZE);
	asyncio.ioSubmit(std::move(io2)).then([] (unique_ptr<IO> io2) {
		assert(io2->getResult() == IO_SIZE);
	});

	/* third write */
	auto io3 = make_unique<IO>(fd, IO_SIZE, 1024, IOType::WRITE);
	std::memset(io3->getIOBuffer(), 'C', IO_SIZE);
	auto f  = asyncio.ioSubmit(std::move(io3));
	auto f1 = f.then([] (unique_ptr<IO> io3) {
		assert(io3->getResult() == IO_SIZE);
	});

	f1.then([&base, &asyncio, &fd] {
		auto io4 = make_unique<IO>(fd, IO_SIZE, 0, IOType::WRITE);
		std::memset(io4->getIOBuffer(), 'D', IO_SIZE);
		auto f = asyncio.ioSubmit(std::move(io4));
		f.then([&base] (unique_ptr<IO> io4) {
			assert(io4->getResult() == IO_SIZE);
			base.terminateLoopSoon();
		});
	});

	base.loopForever();

	close(fd);
}