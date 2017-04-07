#include <memory>
#include <vector>

#include <libaio.h>
#include <folly/futures/Future.h>
#include <folly/io/async/EventBase.h>
#include <folly/io/async/EventHandler.h>

#define PAGE_SIZE 4096

using namespace folly;

using ManagedBuffer = std::unique_ptr<char, void(*)(void*)>;

enum class IOType {
	READ,
	WRITE,
};

class IO {
private:
	ManagedBuffer bufp_;
	uint64_t      offset_;
	size_t        size_;
	ssize_t       result_;
	int           fd_;
	IOType        type_;
	struct iocb   iocb_;

	Promise<ssize_t> p_;
private:
	void prepare();
	ManagedBuffer allocBuffer(size_t size);

public:
	IO(int fd, size_t size, uint64_t offset, IOType type);
	char *getIOBuffer();
	struct iocb *getIOCB();
	Future<ssize_t> getFuture();
	void setComplete(ssize_t result);
};

class AsyncIO {
private:
	io_context_t   context_;
	uint16_t       capacity_;
	int            eventfd_;
public:
	class EventFDHandler : public EventHandler {
	private:
		int       fd_;
		EventBase *basep_;
		AsyncIO   *asynciop_;
	public:
		EventFDHandler(AsyncIO *asynciop, EventBase *basep, int fd) :
				fd_(fd), basep_(basep), asynciop_(asynciop), EventHandler(basep, fd) {
			assert(fd >= 0 && basep);
		}

		void handlerReady(uint16_t events) noexcept {
			assert(events & EventHandler::READ);
			if (events & EventHandler::READ) {
				asynciop_->iosCompleted();
			}
		}
	};

	AsyncIO(uint16_t capacity);
	~AsyncIO();

	void init(EventBase *basep);

	void iosCompleted();

	Future<ssize_t> ioSubmit(IO &io);
	std::vector<Future<ssize_t>> iosSubmit(std::vector<IO> &ios);

private:
	std::unique_ptr<EventFDHandler> handlerp_;
};