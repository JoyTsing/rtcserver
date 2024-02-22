#pragma once
#include "base/event_loop.h"
#include <thread>
namespace xrtc {
class SignalingWorker {
  public:
    using IOWatcher = EventLoop::IOWatcher;
    enum { QUIT };

  public:
    explicit SignalingWorker(int worker_id);
    ~SignalingWorker();

    bool Init();
    bool Start();
    void Stop();
    bool Notify(ssize_t msg);
    void Join();

  public:
    static void WorkerRecvNotify(
        EventLoop *el, IOWatcher *w, int fd, int events, void *data);

  private:
    void HandleNotify(ssize_t msg);
    void StopEvent();

  private:
    int _worker_id;
    EventLoop *_event_loop = nullptr;
    // communicate with main thread
    int _notify_recv_fd = -1;
    int _notify_send_fd = -1;
    IOWatcher *_pipe_watcher = nullptr;
    // thread
    std::thread *_thread = nullptr;
};
} // namespace xrtc
