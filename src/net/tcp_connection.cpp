#include "net/tcp_connection.h"
#include "net/socket.h"
#include "rtc_base/sds.h"
#include <unistd.h>

namespace xrtc {
TcpConnection::TcpConnection(int sock) : socket(sock), read_buf(sdsempty()) {
    SetSockNoBlock(socket);
    SetSockNoDelay(socket);
    SockPeerAddr(socket, addr, port);
}

TcpConnection::~TcpConnection() {
}

} // namespace xrtc