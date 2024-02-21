#pragma once
#include <libev/ev.h>
namespace xrtc {
class EventLoop;
class IOWatcher;
using io_callback_t =
    void (*)(EventLoop *, IOWatcher *, int fd, int events, void *data);

class EventLoop {
  public:
    class IOWatcher {
      public:
        IOWatcher(EventLoop *el, io_callback_t call, void *data)
            : el(el), call(call), data(data) {
            io.data = this;
        }

      public:
        ev_io io;

        EventLoop *el;
        io_callback_t call;
        void *data;
    };

  public:
    explicit EventLoop(void *owner);
    ~EventLoop();

    void start();
    void stop();

    IOWatcher *CreateIoEvent(io_callback_t call, void *data);

  private:
    struct ev_loop *_loop;
    void *owner;
};
} // namespace xrtc