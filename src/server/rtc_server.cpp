
#include "server/rtc_server.h"
#include "base/event_loop.h"
#include "server/rtc_worker.h"
#include "yaml-cpp/node/parse.h"
#include <rtc_base/crc32.h>
#include <rtc_base/logging.h>
#include <unistd.h>
#include <yaml-cpp/yaml.h>

namespace xrtc {
RtcServer::RtcServer() : _event_loop(new EventLoop(this)) {
}

RtcServer::~RtcServer() {
    delete _event_loop;
    delete _thread;

    _event_loop = nullptr;
    _thread = nullptr;

    for (auto worker : _workers) {
        if (worker) { delete worker; }
    }

    _workers.clear();
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

bool RtcServer::SendRtcMessage(std::shared_ptr<RtcMessage> msg) {
    PushMessage(msg);
    return Notify(RTC_MSG);
}

void RtcServer::PushMessage(const std::shared_ptr<RtcMessage> &msg) {
    std::unique_lock<std::mutex> lock(_q_mtx);
    _queue_msg.push(msg);
}

std::shared_ptr<RtcMessage> RtcServer::PopMessage() {
    std::unique_lock<std::mutex> lock(_q_mtx);

    if (_queue_msg.empty()) { return nullptr; }

    std::shared_ptr<RtcMessage> msg = _queue_msg.front();
    _queue_msg.pop();
    return msg;
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
        RTC_LOG(LS_INFO) << "===>rtc server event loop start";
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
            ProcessRtcMsg();
            break;
        default:
            RTC_LOG(LS_WARNING) << "unknown msg: " << msg;
            break;
    }
}

RtcWorker *RtcServer::GetWorker(const std::string &stream_name) {
    if (_workers.empty() || _workers.size() != (size_t)_options.worker_num) {
        return nullptr;
    }
    uint32_t num = rtc::ComputeCrc32(stream_name);
    size_t index = num % _options.worker_num;
    return _workers[index];
}

void RtcServer::ProcessRtcMsg() {
    std::shared_ptr<RtcMessage> msg = PopMessage();
    if (!msg) { return; }
    RtcWorker *worker = GetWorker(msg->stream_name);
    if (worker) { worker->SendRtcMessage(msg); }
}

void RtcServer::StopEvent() {
    _event_loop->DeleteIOEvent(_pipe_watcher);
    _event_loop->Stop();
    close(_notify_recv_fd);
    close(_notify_send_fd);
    for (auto work : _workers) {
        if (work) {
            work->Stop();
            work->Join();
        }
    }
}

bool RtcServer::CreateWorker(int worker_id) {
    RTC_LOG(LS_INFO) << "rtc server create worker, worker_id: " << worker_id;
    auto *worker = new RtcWorker(worker_id, _options);

    if (!worker->Init()) { return false; }

    if (!worker->Start()) { return false; }

    _workers.push_back(worker);

    return true;
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
    // worker
    for (int i = 0; i < _options.worker_num; i++) {
        if (!CreateWorker(i)) {
            RTC_LOG(LS_WARNING) << "create worker failed";
            return false;
        }
    }
    return true;
}

}; // namespace xrtc