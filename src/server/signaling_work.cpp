#include "server/signaling_work.h"
#include "base/event_loop.h"
#include "net/socket.h"
#include "net/tcp_connection.h"
#include "rtc_base/sds.h"
#include <rtc_base/logging.h>
#include <unistd.h>
namespace xrtc {
SignalingWorker::SignalingWorker(int worker_id)
    : _worker_id(worker_id), _event_loop(new EventLoop(this)) {
    _conn_pool = {};
}

SignalingWorker::~SignalingWorker() {
    delete _event_loop;
    _event_loop = nullptr;

    delete _thread;
    _thread = nullptr;
}

bool SignalingWorker::Init() {
    // communicate with main thread
    int fds[2];
    if (pipe(fds) != 0) {
        RTC_LOG(LS_WARNING) << "worker create pipe failed";
        return false;
    }
    _notify_recv_fd = fds[0];
    _notify_send_fd = fds[1];
    // event
    _pipe_watcher = _event_loop->CreateIoEvent(WorkerRecvNotify, this);
    _event_loop->StartIOEvent(_pipe_watcher, _notify_recv_fd, EventLoop::READ);
    return true;
}

bool SignalingWorker::Start() {
    if (_thread) {
        RTC_LOG(LS_WARNING)
            << "worker has already started,worker_id:" << _worker_id;
        return false;
    }
    _thread = new std::thread([=]() {
        RTC_LOG(LS_INFO) << "worker start,worker_id:" << _worker_id;
        _event_loop->Start();
        RTC_LOG(LS_INFO) << "exit from worker event loop, worker_id:"
                         << _worker_id;
    });
    return true;
}

void SignalingWorker::Stop() {
    Notify(SignalingWorker::QUIT);
}

bool SignalingWorker::Notify(ssize_t msg) {
    ssize_t write_len = write(_notify_send_fd, &msg, sizeof(msg));
    return write_len == sizeof(msg);
}

bool SignalingWorker::NotifyNewConnection(int fd) {
    _conn_queue.Produce(fd);
    return Notify(SignalingWorker::NEW_CONNECTION);
}

void SignalingWorker::WorkerRecvNotify(
    EventLoop * /*el*/, IOWatcher * /*w*/, int fd, int /*events*/, void *data) {
    ssize_t msg;
    if (read(fd, &msg, sizeof(msg)) != sizeof(msg)) {
        RTC_LOG(LS_WARNING) << "read notify msg failed";
        return;
    }
    auto *worker = (SignalingWorker *)data;
    worker->HandleNotify(msg);
}

void SignalingWorker::ConnectIOCall(
    EventLoop * /*el*/, IOWatcher * /*w*/, int fd, int events, void *data) {
    auto *worker = (SignalingWorker *)data;
    if (events == EventLoop::READ) { worker->ReadEvent(fd); }
}

void SignalingWorker::ReadEvent(int fd) {
    RTC_LOG(LS_INFO) << "worker read event, worker_id:" << _worker_id
                     << ", fd:" << fd;
    if (fd < 0) {
        RTC_LOG(LS_WARNING) << "invalid fd: " << fd;
        return;
    }
    TcpConnection *conn = _conn_pool[fd];
    // 36 byte header
    int read_len = conn->kHeadBytes;
    int buf_len = sdslen(conn->read_buf);
    conn->read_buf = sdsMakeRoomFor(conn->read_buf, read_len);

    // read
    ssize_t nread = SocketReadData(fd, conn->read_buf + buf_len, read_len);

    if (nread < 0) {
        RTC_LOG(LS_WARNING) << "read failed, fd:" << fd;
        return;
    }
    if (nread > 0) { sdsIncrLen(conn->read_buf, nread); }
    RTC_LOG(LS_INFO) << "sock read data,len:" << nread;
}

void SignalingWorker::HandleNotify(ssize_t msg) {
    if (msg == SignalingWorker::QUIT) {
        StopEvent();
        return;
    }
    if (msg == SignalingWorker::NEW_CONNECTION) {
        int conn_fd;
        if (_conn_queue.Consume(conn_fd)) { NewConnection(conn_fd); }
        return;
    }
}

void SignalingWorker::StopEvent() {
    if (!_thread) {
        RTC_LOG(LS_WARNING)
            << "signaling worker has already stopped, worker_id:" << _worker_id;
        return;
    }
    _event_loop->DeleteIOEvent(_pipe_watcher);
    _event_loop->Stop();

    close(_notify_recv_fd);
    close(_notify_send_fd);

    RTC_LOG(LS_INFO) << "signaling worker stop";
}

void SignalingWorker::NewConnection(int fd) {
    RTC_LOG(LS_INFO) << "new connection, worker_id:" << _worker_id
                     << ", fd:" << fd;
    if (fd < 0) {
        RTC_LOG(LS_WARNING) << "invalid fd: " << fd;
        return;
    }

    auto *conn = new TcpConnection(fd);
    conn->io_watcher = _event_loop->CreateIoEvent(ConnectIOCall, this);
    _event_loop->StartIOEvent(conn->io_watcher, fd, EventLoop::READ);
    _conn_pool[fd] = conn;
}

void SignalingWorker::Join() {
    if (_thread && _thread->joinable()) { _thread->join(); }
}

} // namespace xrtc