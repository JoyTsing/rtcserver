#pragma once

#include <libev/ev.h>
#include <cstdint>
namespace xrtc {
class EventLoop {
  public:
    enum { READ = 0x1, WRITE = 0x2 };

  public:
    class IOWatcher;
    using io_callback_t =
        void (*)(EventLoop *, IOWatcher *, int fd, int events, void *data);

    class IOWatcher {
      public:
        IOWatcher(EventLoop *el, io_callback_t call, void *data)
            : event_loop(el), call(call), data(data) {
            io = new ev_io{};
            io->data = this;
        }

      public:
        ev_io *io;

        EventLoop *event_loop;
        io_callback_t call;
        void *data;
    };

  public:
    class TimeWatcher;
    using time_callback_t = void (*)(EventLoop *, TimeWatcher *w, void *data);

    class TimeWatcher {
      public:
        TimeWatcher(
            EventLoop *event_loop,
            time_callback_t call,
            void *data,
            bool repeate)
            : event_loop(event_loop), call(call), data(data), repeate(repeate) {
            timer = new ev_timer;
            timer->data = this;
        }

      public:
        EventLoop *event_loop;
        struct ev_timer *timer;
        time_callback_t call;
        void *data;
        bool repeate;
    };

  public:
    explicit EventLoop(void *owner);
    ~EventLoop();

    void *GetOwner() { return owner; }

    void Start();
    void Stop();

    // time
    uint64_t Now();

    IOWatcher *CreateIoEvent(io_callback_t call, void *data);
    void StartIOEvent(IOWatcher *w, int fd, int mask);
    void StopIOEvent(IOWatcher *w, int fd, int mask);
    void DeleteIOEvent(IOWatcher *w);

    TimeWatcher *
    CreateTimerEvent(time_callback_t call, void *data, bool repeate);
    void StartTimerEvent(TimeWatcher *w, uint64_t usec);
    void StopTimerEvent(TimeWatcher *w);
    void DeleteTimerEvent(TimeWatcher *w);

  private:
    static void
    generic_io_callback(struct ev_loop *loop, struct ev_io *w, int events);

    static void generic_time_callback(
        struct ev_loop *loop, struct ev_timer *timer, int events);

  private:
    struct ev_loop *_loop;
    void *owner;
};
} // namespace xrtc