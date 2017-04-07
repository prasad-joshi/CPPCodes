#include <memory>
#include <vector>

#include <libaio.h>
#include <sys/eventfd.h>

#include <folly/futures/Future.h>
#include <folly/io/async/EventBase.h>
#include <folly/io/async/EventHandler.h>

#include "AsyncIO.h"

using namespace folly;
using std::unique_ptr;
using std::vector;
using std::make_unique;


IO::IO(int fd, size_t size, uint64_t offset, IOType type) :
		fd_(fd), size_(size), offset_(offset), type_(type), bufp_(allocBuffer(size)) {
	prepare();
	assert(p_.isFulfilled() == false && bufp_);
}

void IO::prepare() {
	switch (type_) {
	case IOType::READ:
		io_prep_pread(&iocb_, fd_, reinterpret_cast<void *>(bufp_.get()), size_, offset_);
		break;
	case IOType::WRITE:
		io_prep_pwrite(&iocb_, fd_, reinterpret_cast<void *>(bufp_.get()), size_, offset_);
	}
}

ManagedBuffer IO::allocBuffer(size_t size) {
	void *bufp{nullptr};
	auto rc = posix_memalign(&bufp, PAGE_SIZE, size);
	assert(rc == 0 && bufp);
	return ManagedBuffer(reinterpret_cast<char *>(bufp), free);
}

char *IO::getIOBuffer() {
	return bufp_.get();
}

struct iocb *IO::getIOCB() {
	return &iocb_;
}

void IO::setComplete(ssize_t result) {
	assert(p_.isFulfilled() == false);
	p_.setValue(result);
}

Future<ssize_t> IO::getFuture() {
	return p_.getFuture();
}

AsyncIO::AsyncIO(uint16_t capacity) : capacity_(capacity) {
	std::memset(&context_, 0, sizeof(context_));

	auto rc = io_setup(capacity, &context_);
	assert(rc == 0);
}

AsyncIO::~AsyncIO() {
	io_destroy(context_);
}

void AsyncIO::init(EventBase *basep) {
	eventfd_ = eventfd(0, EFD_NONBLOCK);
	assert(eventfd_ >= 0);

	handlerp_ = make_unique<EventFDHandler>(this, basep, eventfd_);
	assert(handlerp_);
	handlerp_->registerHandler(EventHandler::READ | EventHandler::PERSIST);
}

Future<ssize_t> AsyncIO::ioSubmit(IO &io) {
	struct iocb *iocb[1];
	auto iocbp  = io.getIOCB();
	iocbp->data = reinterpret_cast<void *>(&io);
	iocb[0]     =  iocbp;
	io_set_eventfd(iocbp, eventfd_);
	io_submit(context_, 1, iocb);
	return io.getFuture();
}

vector<Future<ssize_t>> AsyncIO::iosSubmit(vector<IO> &ios) {
	vector<Future<ssize_t>> v;

	size_t nios = ios.size();
	struct iocb *iocbs[nios];
	uint32_t i = 0;
	for (auto &io : ios) {
		auto iocbp  = io.getIOCB();
		iocbp->data = reinterpret_cast<void *>(&io);
		iocbs[i]    =  iocbp;
		io_set_eventfd(iocbp, eventfd_);
		i++;
		v.emplace_back(io.getFuture());
	}
	io_submit(context_, nios, iocbs);

	return v;
}

ssize_t ioResult(struct io_event *ep) {
	return ((ssize_t)(((uint64_t)ep->res2 << 32) | ep->res));
}

void AsyncIO::iosCompleted() {
	assert(eventfd_ > 0);

	while (1) {
		eventfd_t nevents;
		auto rc = eventfd_read(eventfd_, &nevents);
		if (rc < 0 || nevents == 0) {
			if (rc < 0 && errno != EAGAIN) {
				assert(0);
			}
			assert(errno == EAGAIN);
			break;
		}

		assert(nevents > 0);
		struct io_event events[nevents];
		rc = io_getevents(context_, nevents, nevents, events, NULL);
		assert(rc == nevents);

		for (auto ep = events; ep < events + nevents; ep++) {
			auto *iop   = reinterpret_cast<IO*>(ep->data);
			auto result = ioResult(ep);
			iop->setComplete(result);
		}
	}
}
