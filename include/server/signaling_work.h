#pragma once
#include "base/event_loop.h"
#include "base/lock_free_queue.h"
#include <thread>
namespace xrtc {
class SignalingWorker {
  public:
    using IOWatcher = EventLoop::IOWatcher;
    enum { QUIT, NEW_CONNECTION };

  public:
    explicit SignalingWorker(int worker_id);
    ~SignalingWorker();

    bool Init();
    bool Start();
    void Stop();
    void Join();

    bool Notify(ssize_t msg);
    bool NotifyNewConnection(int fd);

  private:
    static void WorkerRecvNotify(
        EventLoop *el, IOWatcher *w, int fd, int events, void *data);

  private:
    void HandleNotify(ssize_t msg);
    void StopEvent();
    void NewConnection(int fd);

  private:
    int _worker_id;
    EventLoop *_event_loop = nullptr;
    LockFreeQueue<int> _conn_queue;
    // communicate with main thread
    int _notify_recv_fd = -1;
    int _notify_send_fd = -1;
    IOWatcher *_pipe_watcher = nullptr;
    // thread
    std::thread *_thread = nullptr;
};
} // namespace xrtc
