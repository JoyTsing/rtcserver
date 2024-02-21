#pragma once
#include "base/event_loop.h"
#include <string>
#include <thread>
#include <unistd.h>

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

    int Init(const std::string &conf_file);
    bool Start();
    void Stop();
    bool Notify(ssize_t msg);
    void Join();

  public:
    static void AcceptNewConnection(
        EventLoop *el, IOWatcher *w, int fd, int events, void *data);
    static void ServerRecvNotify(
        EventLoop *el, IOWatcher *w, int fd, int events, void *data);

  private:
    void HandleNotify(ssize_t msg);
    void StopEvent();

  private:
    SignalingServerOptions _options;
    EventLoop *_event_loop;
    IOWatcher *_io_watcher = nullptr;
    IOWatcher *_pipe_watcher = nullptr;
    // net
    int _listen_fd = -1;
    // control event
    int _notify_recv_fd = -1;
    int _notify_send_fd = -1;
    // thread
    std::thread *_thread = nullptr;
};

} // namespace xrtc