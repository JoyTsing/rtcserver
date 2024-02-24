#include "server/rtc_worker.h"
#include "base/event_loop.h"
#include <rtc_base/logging.h>
#include <unistd.h>
namespace xrtc {

RtcWorker::RtcWorker(int worker_id, const RtcServerOptions &options)
    : _event_loop(new EventLoop(this))
    , _options(options)
    , _worker_id(worker_id) {
}

RtcWorker::~RtcWorker() {
    delete _event_loop;
    delete _thread;

    _event_loop = nullptr;
    _thread = nullptr;
}

void RtcWorker::PushMessage(const std::shared_ptr<RtcMessage> &msg) {
    _q_msg.Produce(msg);
}

bool RtcWorker::PopMessage(std::shared_ptr<RtcMessage> &msg) {
    return _q_msg.Consume(msg);
}

void RtcWorker::WorkerRecvNotify(
    EventLoop * /*el*/, IOWatcher * /*w*/, int fd, int /*events*/, void *data) {
    ssize_t msg;
    if (read(fd, &msg, sizeof(msg)) != sizeof(msg)) {
        RTC_LOG(LS_WARNING) << "read from pipe error, errno: " << errno;
        return;
    }

    auto *worker = (RtcWorker *)data;
    worker->HandleNotify(msg);
}

bool RtcWorker::SendRtcMessage(std::shared_ptr<RtcMessage> msg) {
    PushMessage(msg);
    return Notify(RTC_MSG);
}

bool RtcWorker::Init() {
    int fds[2];
    if (pipe(fds)) {
        RTC_LOG(LS_WARNING) << "create pipe error, errno: " << errno;
        return -1;
    }

    _notify_recv_fd = fds[0];
    _notify_send_fd = fds[1];

    _pipe_watcher = _event_loop->CreateIoEvent(WorkerRecvNotify, this);
    _event_loop->StartIOEvent(_pipe_watcher, _notify_recv_fd, EventLoop::READ);
    return true;
}

bool RtcWorker::Start() {
    if (_thread) {
        RTC_LOG(LS_WARNING)
            << "rtc worker already start, worker_id: " << _worker_id;
        return false;
    }

    _thread = new std::thread([=]() {
        RTC_LOG(LS_INFO) << "rtc worker event loop start, worker_id: "
                         << _worker_id;
        _event_loop->Start();
        RTC_LOG(LS_INFO) << "rtc worker event loop stop, worker_id: "
                         << _worker_id;
    });
    return true;
}

void RtcWorker::Stop() {
    Notify(QUIT);
}

void RtcWorker::Join() {
    if (_thread && _thread->joinable()) { _thread->join(); }
}

void RtcWorker::StopEvent() {
    if (!_thread) {
        RTC_LOG(LS_WARNING)
            << "rtc worker not running, worker_id: " << _worker_id;
        return;
    }

    _event_loop->DeleteIOEvent(_pipe_watcher);
    _event_loop->Stop();
    close(_notify_recv_fd);
    close(_notify_send_fd);
}

void RtcWorker::ProcessPush(std::shared_ptr<RtcMessage> msg) {
    // std::string offer = "offer";

    // msg->sdp = offer;

    // auto *worker = (SignalingWorker *)(msg->worker);
    // if (worker) { worker->send_rtc_msg(msg); }
}

void RtcWorker::ProcessRtcMsg() {
    std::shared_ptr<RtcMessage> msg;
    if (!PopMessage(msg)) { return; }

    RTC_LOG(LS_INFO) << "cmdno[" << msg->cmdno << "] uid[" << msg->uid
                     << "] stream_name[" << msg->stream_name << "] audio["
                     << msg->audio << "] video[" << msg->video << "] log_id["
                     << msg->log_id
                     << "] rtc worker receive msg, worker_id: " << _worker_id;

    switch (msg->cmdno) {
        case CMDNO_PUSH:
            ProcessPush(msg);
            break;
        default:
            RTC_LOG(LS_WARNING) << "unknown cmdno: " << msg->cmdno
                                << ", log_id: " << msg->log_id;
            break;
    }
}

void RtcWorker::HandleNotify(int msg) {
    switch (msg) {
        case QUIT:
            StopEvent();
            break;
        case RTC_MSG:
            //
            ProcessRtcMsg();
            break;
        default:
            RTC_LOG(LS_WARNING) << "unknown msg: " << msg;
            break;
    }
}

bool RtcWorker::Notify(ssize_t msg) {
    ssize_t write_len = write(_notify_send_fd, &msg, sizeof(msg));
    return write_len == sizeof(msg);
}

}; // namespace xrtc