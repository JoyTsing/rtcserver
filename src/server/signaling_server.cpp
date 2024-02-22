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
    delete _thread;
    _thread = nullptr;
    delete _event_loop;
    _event_loop = nullptr;

    for (auto &worker : _workers) { delete worker; }
    _workers.clear();
}

bool SignalingServer::CreateWorker(int i) {
    auto *worker = new SignalingWorker(i);
    RTC_LOG(LS_INFO) << "create worker, worker_id:" << i;
    if (!worker->Init()) {
        RTC_LOG(LS_WARNING) << "worker init failed, worker_id:" << i;
        return false;
    }
    if (!worker->Start()) {
        RTC_LOG(LS_WARNING) << "worker start failed, worker_id:" << i;
        return false;
    }
    _workers.emplace_back(worker);
    return true;
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
    RTC_LOG(LS_INFO) << "signaling server start stop";
    _event_loop->DeleteIOEvent(_pipe_watcher);
    _event_loop->DeleteIOEvent(_io_watcher);
    _event_loop->Stop();

    close(_notify_recv_fd);
    close(_notify_send_fd);
    close(_listen_fd);
    for (auto &worker : _workers) {
        if (worker) {
            worker->Stop();
            worker->Join();
            delete worker;
        }
    }
}

void SignalingServer::AcceptNewConnection(
    EventLoop * /*el*/, IOWatcher * /*w*/, int fd, int /*events*/, void *data) {
    auto *server = (SignalingServer *)data;
    std::string client_ip{};
    int conn_fd, conn_port;
    conn_fd = TCPAccept(fd, client_ip, conn_port);
    if (conn_fd < 0) {
        RTC_LOG(LS_WARNING) << "accept new connection failed";
        return;
    }
    RTC_LOG(LS_INFO) << "accept new connection, fd:" << conn_fd
                     << ", ip:" << client_ip.data() << ", port:" << conn_port;
    server->DispatchConnection(conn_fd);
}

void SignalingServer::DispatchConnection(int conn_fd) {
    int index = _next_worker_index++;
    if (_next_worker_index >= _options.worker_num) { _next_worker_index = 0; }
    _workers[index]->NotifyNewConnection(conn_fd);
}

void SignalingServer::ServerRecvNotify(
    EventLoop * /*el*/, IOWatcher * /*w*/, int fd, int /*events*/, void *data) {
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
    if (_listen_fd < 0) {
        RTC_LOG(LS_WARNING) << "create listenfd failed";
        return -1;
    }
    // event_loop
    _io_watcher = _event_loop->CreateIoEvent(AcceptNewConnection, this);
    _event_loop->StartIOEvent(_io_watcher, _listen_fd, EventLoop::READ);
    // worker
    for (int i = 0; i < _options.worker_num; i++) {
        if (!CreateWorker(i)) {
            RTC_LOG(LS_WARNING) << "create worker failed";
            return -1;
        }
    }
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
