#include "net/tcp_connection.h"
#include "net/socket.h"
#include "net/tcp_head.h"
#include "rtc_base/sds.h"
#include <unistd.h>

namespace xrtc {
TcpConnection::TcpConnection(int sock)
    : socket(sock)
    , expected_bytes(kHeadSize)
    , process_bytes(0)
    , read_buf(sdsempty()) {
    SetSockNoBlock(socket);
    SetSockNoDelay(socket);
    SockPeerAddr(socket, addr, port);
}

TcpConnection::~TcpConnection() {
}

} // namespace xrtc