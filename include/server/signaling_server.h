#pragma once
#include "base/event_loop.h"
#include "server/signaling_work.h"
#include <string>
#include <thread>
#include <unistd.h>
#include <vector>

namespace xrtc {

struct SignalingServerOptions {
    std::string host;
    int port;
    int worker_num;
    int connection_timeout;
};

class SignalingServer {
  public:
    using IOWatcher = EventLoop::IOWatcher;
    enum { QUIT = 0 };

  public:
    SignalingServer();
    ~SignalingServer();

    bool Init(const std::string &conf_file);
    bool Start();
    void Stop();
    bool Notify(ssize_t msg);
    void Join();

  private:
    static void AcceptNewConnection(
        EventLoop *el, IOWatcher *w, int fd, int events, void *data);
    static void ServerRecvNotify(
        EventLoop *el, IOWatcher *w, int fd, int events, void *data);

  private:
    void StopEvent();
    bool CreateWorker(int i);
    void HandleNotify(ssize_t msg);
    void DispatchConnection(int conn_fd);

  private:
    SignalingServerOptions _options;
    EventLoop *_event_loop;
    IOWatcher *_io_watcher = nullptr;
    IOWatcher *_pipe_watcher = nullptr;
    // net
    int _listen_fd = -1;
    int _next_worker_index = 0;
    // control event
    int _notify_recv_fd = -1;
    int _notify_send_fd = -1;
    // worker
    std::vector<SignalingWorker *> _workers;
    // thread
    std::thread *_thread = nullptr;
};

} // namespace xrtc