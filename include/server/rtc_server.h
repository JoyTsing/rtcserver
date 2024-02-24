#pragma once

#include "base/event_loop.h"
#include <string>
#include <thread>
#include <unistd.h>
namespace xrtc {
struct RtcServerOptions {
    int worker_num;
};
class RtcServer {
  public:
    enum {
        QUIT = 0,
        RTC_MSG = 1,
    };
    using IOWatcher = EventLoop::IOWatcher;

  public:
    RtcServer();
    ~RtcServer();

    bool Init(const std::string &file);
    bool Start();
    void Stop();
    void Join();

    bool Notify(ssize_t msg);

  private:
    static void ServerRecvNotify(
        EventLoop *el, IOWatcher *w, int fd, int events, void *data);

  private:
    void StopEvent();
    void HandleNotify(ssize_t msg);

  private:
    EventLoop *_event_loop;
    RtcServerOptions _options;

    IOWatcher *_pipe_watcher = nullptr;

    int _notify_recv_fd = -1;
    int _notify_send_fd = -1;
    // thread
    std::thread *_thread = nullptr;
};

} // namespace xrtc