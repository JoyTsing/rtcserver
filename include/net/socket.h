#pragma once
#include <string>
#include <sys/socket.h>
namespace xrtc {
int CreateTcpServer(const std::string &addr, int port);
int TCPAccept(int sock, std::string &addr, int &port);
int Accept(int sock, struct sockaddr *sa, socklen_t *len);

bool SetSockNoBlock(int sock);
bool SetSockNoDelay(int sock);

bool SockPeerAddr(int sock, std::string &addr, int &port);

} // namespace xrtc