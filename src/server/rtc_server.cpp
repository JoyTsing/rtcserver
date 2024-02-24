
#include "server/rtc_server.h"
#include "base/event_loop.h"
#include "yaml-cpp/node/parse.h"
#include <unistd.h>
#include <yaml-cpp/yaml.h>
#include <rtc_base/logging.h>

namespace xrtc {
RtcServer::RtcServer() : _event_loop(new EventLoop(this)) {
}

RtcServer::~RtcServer() {
}

void RtcServer::ServerRecvNotify(
    EventLoop * /*el*/, IOWatcher * /*w*/, int fd, int /*events*/, void *data) {
    ssize_t msg;
    if (read(fd, &msg, sizeof(msg)) != sizeof(msg)) {
        RTC_LOG(LS_WARNING) << "read from pipe error, errno: " << errno;
        return;
    }

    auto *server = (RtcServer *)data;
    server->HandleNotify(msg);
}

void RtcServer::Stop() {
    Notify(QUIT);
}

bool RtcServer::Notify(ssize_t msg) {
    ssize_t write_len = write(_notify_send_fd, &msg, sizeof(msg));
    return write_len == sizeof(msg);
}

void RtcServer::Join() {
    if (_thread && _thread->joinable()) { _thread->join(); }
}

bool RtcServer::Start() {
    if (_thread) {
        RTC_LOG(LS_WARNING) << "rtc server already start";
        return false;
    }

    _thread = new std::thread([=]() {
        RTC_LOG(LS_INFO) << "rtc server event loop start";
        _event_loop->Start();
        RTC_LOG(LS_INFO) << "rtc server event loop stop";
    });

    return true;
}

void RtcServer::RtcServer::HandleNotify(ssize_t msg) {
    switch (msg) {
        case QUIT:
            StopEvent();
            break;
        case RTC_MSG:
            // ProcessRtcMsg();
            break;
        default:
            RTC_LOG(LS_WARNING) << "unknown msg: " << msg;
            break;
    }
}

void RtcServer::StopEvent() {
    _event_loop->DeleteIOEvent(_pipe_watcher);
    _event_loop->Stop();
    close(_notify_recv_fd);
    close(_notify_send_fd);
}

bool RtcServer::Init(const std::string &file) {
    try {
        YAML::Node config = YAML::LoadFile(file);
        _options.worker_num = config["server"]["worker_num"].as<int>();
    } catch (YAML::Exception &e) {
        RTC_LOG(LS_WARNING) << "rtc server load conf file error: " << e.msg;
        return false;
    }
    int fds[2];
    if (pipe(fds)) {
        RTC_LOG(LS_WARNING) << "create pipe error, errno: " << errno;
        return -1;
    }

    _notify_recv_fd = fds[0];
    _notify_send_fd = fds[1];

    _pipe_watcher = _event_loop->CreateIoEvent(ServerRecvNotify, this);
    _event_loop->StartIOEvent(_pipe_watcher, _notify_recv_fd, EventLoop::READ);
    return true;
}

}; // namespace xrtc