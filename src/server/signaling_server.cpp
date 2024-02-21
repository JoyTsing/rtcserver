#include "server/signaling_server.h"
#include "base/event_loop.h"
#include "net/socket.h"
#include "rtc_base/logging.h"
#include <cerrno>
#include <thread>
#include <unistd.h>
#include <yaml-cpp/yaml.h>

namespace xrtc {

SignalingServer::SignalingServer() : _event_loop(new EventLoop(this)) {
}

SignalingServer::~SignalingServer() {
}
void SignalingServer::Join() {
    if (_thread && _thread->joinable()) { _thread->join(); }
}

void SignalingServer::HandleNotify(ssize_t msg) {
    if (msg == SignalingServer::QUIT) {
        StopEvent();
        return;
    }
}

void SignalingServer::StopEvent() {
    if (!_thread) {
        RTC_LOG(LS_WARNING) << "signaling server has already stopped";
        return;
    }
    _event_loop->DeleteIOEvent(_pipe_watcher);
    _event_loop->DeleteIOEvent(_io_watcher);
    _event_loop->Stop();

    close(_notify_recv_fd);
    close(_notify_send_fd);
    close(_listen_fd);
    RTC_LOG(LS_INFO) << "signaling server stop";
}

void SignalingServer::AcceptNewConnection(
    EventLoop *el, IOWatcher *w, int fd, int events, void *data) {
}

void SignalingServer::ServerRecvNotify(
    EventLoop *el, IOWatcher *w, int fd, int events, void *data) {
    ssize_t msg;
    if (read(fd, &msg, sizeof(msg)) != sizeof(msg)) {
        RTC_LOG(LS_WARNING) << "read notify msg failed";
        return;
    }
    auto *server = (SignalingServer *)data;
    server->HandleNotify(msg);
}

int SignalingServer::Init(const std::string &conf_file) {
    if (conf_file.empty()) {
        RTC_LOG(LS_WARNING) << "signaling server conf file is empty";
        return -1;
    }
    // config
    YAML::Node config = YAML::LoadFile(conf_file);
    try {
        _options.host = config["server"]["host"].as<std::string>();
        _options.port = config["server"]["port"].as<int>();
        _options.worker_num = config["server"]["worker_num"].as<int>();
        _options.connection_timeout =
            config["server"]["connection_timeout"].as<int>();
    } catch (YAML::Exception &e) {
        RTC_LOG(LS_WARNING)
            << "catch a YAML exception in signaling, message:" << e.msg;
        return -1;
    }
    // thread communication
    int fds[2];
    if (pipe(fds)) {
        RTC_LOG(LS_WARNING) << "create pipe failed: " << errno;
        return -1;
    }
    _notify_recv_fd = fds[0];
    _notify_send_fd = fds[1];
    // recv_fd add to event loop
    _pipe_watcher = _event_loop->CreateIoEvent(ServerRecvNotify, this);
    _event_loop->StartIOEvent(_pipe_watcher, _notify_recv_fd, EventLoop::READ);
    // net
    _listen_fd = CreateTcpServer(_options.host, _options.port);
    // event_loop
    _io_watcher = _event_loop->CreateIoEvent(AcceptNewConnection, this);
    _event_loop->StartIOEvent(_io_watcher, _listen_fd, EventLoop::READ);
    return 0;
}

bool SignalingServer::Start() {
    if (_thread) {
        RTC_LOG(LS_WARNING) << "signaling server has already started";
        return false;
    }
    _thread = new std::thread([=]() {
        RTC_LOG(LS_INFO) << "signaling server start";
        _event_loop->Start();
        RTC_LOG(LS_INFO) << "exit from server event loop";
    });
    return true;
}

void SignalingServer::Stop() {
    Notify(SignalingServer::QUIT);
}

bool SignalingServer::Notify(ssize_t msg) {
    ssize_t write_len = write(_notify_send_fd, &msg, sizeof(msg));
    return write_len == sizeof(msg);
}
} // namespace xrtc
