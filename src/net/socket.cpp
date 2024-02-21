#include "net/socket.h"
#include "rtc_base/logging.h"
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

namespace xrtc {
int CreateTcpServer(const std::string &addr, int port) {
    // create
    int socket = ::socket(AF_INET, SOCK_STREAM, 0);
    if (socket < 0) {
        RTC_LOG(LS_WARNING) << "create socket failed, error:" << errno;
        return -1;
    }
    // set
    int optval = 1;
    int ret =
        setsockopt(socket, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
    if (ret < 0) {
        RTC_LOG(LS_WARNING) << "setsockopt failed, error:" << errno;
        return -1;
    }
    // create addr
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = inet_addr(addr.c_str());

    // bind
    ret = ::bind(socket, (struct sockaddr *)&server_addr, sizeof(server_addr));
    if (ret < 0) {
        RTC_LOG(LS_WARNING) << "bind failed, error:" << errno;
        close(socket);
        return -1;
    }

    // listen
    ret = listen(socket, 4095);
    if (ret < 0) {
        RTC_LOG(LS_WARNING) << "listen failed, error:" << errno;
        close(socket);
        return -1;
    }
    return socket;
}
} // namespace xrtc