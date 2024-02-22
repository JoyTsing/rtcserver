#include "net/socket.h"
#include "rtc_base/logging.h"
#include <arpa/inet.h>
#include <fcntl.h>
#include <mutex>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <unistd.h>

namespace xrtc {
namespace {
    std::mutex __net_mutex;
} // namespace

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

int TCPAccept(int sock, std::string &addr, int &port) {
    struct sockaddr_in client_addr;
    socklen_t len = sizeof(client_addr);
    int client_sock = Accept(sock, (struct sockaddr *)&client_addr, &len);
    if (client_sock < 0) {
        RTC_LOG(LS_WARNING) << "TCP Accept failed, error:" << errno;
        return -1;
    }
    {
        std::lock_guard<std::mutex> lock(__net_mutex);
        strcpy(addr.data(), inet_ntoa(client_addr.sin_addr));
    }
    port = ntohs(client_addr.sin_port);

    return client_sock;
}

int Accept(int sock, struct sockaddr *sa, socklen_t *len) {
    int fd = -1;
    while (true) {
        fd = accept(sock, sa, (socklen_t *)len);
        if (fd < 0) {
            if (errno == EINTR) { continue; } // interrupted by signal
            RTC_LOG(LS_WARNING) << "accept failed, error:" << errno;
            return -1;
        }
        break;
    }
    return fd;
}

bool SetSockNoBlock(int sock) {
    int flags = fcntl(sock, F_GETFL);
    if (flags < 0) {
        RTC_LOG(LS_WARNING)
            << "fcntl(F_GETFL) failed, error:" << errno << ",fd: " << sock;
        return false;
    }

    if (fcntl(sock, F_SETFL, flags | O_NONBLOCK) < 0) {
        RTC_LOG(LS_WARNING)
            << "fcntl(F_SETFL) failed, error:" << errno << ", fd: " << sock;
        return false;
    }
    return true;
}

bool SetSockNoDelay(int sock) {
    int flag = 1;
    int ret = setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, &flag, sizeof(flag));
    if (ret < 0) {
        RTC_LOG(LS_WARNING) << "setsockopt(TCP_NODELAY) failed, error:" << errno
                            << ", fd: " << sock;
        return false;
    }
    return true;
}

bool SockPeerAddr(int sock, std::string &addr, int &port) {
    struct sockaddr_in client_addr;
    socklen_t len;
    if (-1 == getpeername(sock, (struct sockaddr *)&client_addr, &len)) {
        RTC_LOG(LS_WARNING) << "getpeername failed, error:" << errno;
        addr[0] = '?';
        addr[1] = '\0';
        port = 0;
        return false;
    }
    {
        std::lock_guard<std::mutex> lock(__net_mutex);
        strcpy(addr.data(), inet_ntoa(client_addr.sin_addr));
    }
    port = ntohs(client_addr.sin_port);
    return true;
}

ssize_t SocketReadData(int sock, char *buf, size_t len) {
    ssize_t nread = read(sock, buf, len);
    if (nread == 0) {
        RTC_LOG(LS_WARNING) << "peer connection closed,fd: " << sock;
        return -1;
    }
    if (nread < 0 && errno != EAGAIN) {
        RTC_LOG(LS_WARNING) << "socket read failed, error:" << errno;
        return -1;
    }
    if (nread < 0 && errno == EAGAIN) { nread = 0; }
    return nread;
}

} // namespace xrtc