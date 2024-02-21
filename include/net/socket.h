#pragma once
#include <string>
#include <sys/socket.h>
namespace xrtc {
int CreateTcpServer(const std::string &addr, int port);
} // namespace xrtc