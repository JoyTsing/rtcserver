#pragma once
#include "base/event_loop.h"
#include "base/lock_free_queue.h"
#include "net/tcp_connection.h"
#include "rtc_base/slice.h"
#include <thread>
#include <unistd.h>
#include <unordered_map>

namespace xrtc {
class SignalingWorker {
  public:
    using IOWatcher = EventLoop::IOWatcher;
    using TimerWatcher = EventLoop::TimeWatcher;
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

    // timer
    void SetTimeOut(uint64_t usec);

  private:
    static void
    ConnectIOCall(EventLoop *el, IOWatcher *w, int fd, int events, void *data);
    static void ConnectTimeCall(EventLoop *el, TimerWatcher *w, void *data);

    static void WorkerRecvNotify(
        EventLoop *el, IOWatcher *w, int fd, int events, void *data);

  private:
    void HandleNotify(ssize_t msg);
    void StopEvent();
    void NewConnection(int fd);
    // event
    void ReadEvent(int fd);
    bool ProcessRequest(
        TcpConnection *conn, const rtc::Slice &head, const rtc::Slice &body);
    // net
    void CloseConnection(TcpConnection *conn);
    void RemoveConnection(TcpConnection *conn);

    // msg
    bool ProcessReadBuffer(TcpConnection *conn);

    void ProcessTimeout(TcpConnection *conn);

  private:
    // option
    int _worker_id;
    unsigned long _timeout;

    EventLoop *_event_loop = nullptr;
    LockFreeQueue<int> _conn_queue;
    // communicate with main thread
    int _notify_recv_fd = -1;
    int _notify_send_fd = -1;
    std::unordered_map<int, TcpConnection *> _conn_pool;
    IOWatcher *_pipe_watcher = nullptr;
    // thread
    std::thread *_thread = nullptr;
};
} // namespace xrtc
