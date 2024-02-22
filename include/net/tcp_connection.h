#pragma once
#include "base/event_loop.h"
#include "net/tcp_head.h"
#include <rtc_base/sds.h>
#include <string>

namespace xrtc {
struct TcpConnection {
    enum {
        STATE_HEAD,
        STATE_BODY,
    };

    explicit TcpConnection(int sock);
    ~TcpConnection();

    int socket;
    std::string addr;
    int port;
    EventLoop::IOWatcher *io_watcher = nullptr;
    int current_state = STATE_HEAD;

    size_t expected_bytes;
    size_t process_bytes;
    // buf
    sds read_buf;
};
} // namespace xrtc