#pragma once
#include "base/event_loop.h"
#include "net/tcp_head.h"
#include <rtc_base/sds.h>
#include <string>

namespace xrtc {
struct TcpConnection {
    explicit TcpConnection(int sock);
    ~TcpConnection();

    int socket;
    std::string addr;
    int port;
    EventLoop::IOWatcher *io_watcher = nullptr;

    // buf
    sds read_buf;
    size_t process_bytes = 0;
    // const
    const size_t kHeadBytes = kHeadSize;
};
} // namespace xrtc