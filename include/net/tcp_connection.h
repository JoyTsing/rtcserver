#pragma once
#include "base/event_loop.h"
#include <string>
namespace xrtc {
struct TcpConnection {
    explicit TcpConnection(int sock);
    ~TcpConnection();

    int socket;
    std::string addr;
    int port;
    EventLoop::IOWatcher *io_watcher = nullptr;
};
} // namespace xrtc