#pragma once
#include "base/event_loop.h"
#include "base/lock_free_queue.h"
#include "net/server_def.h"
#include "net/tcp_connection.h"
#include "rtc_base/slice.h"
#include <json/json.h>
#include <mutex>
#include <queue>
#include <thread>
#include <unistd.h>
#include <unordered_map>

namespace xrtc {
class SignalingWorker {
  public:
    using IOWatcher = EventLoop::IOWatcher;
    using TimerWatcher = EventLoop::TimeWatcher;
    enum { QUIT = 0, NEW_CONNECTION = 1, RTC_MSG = 2 };

  public:
    explicit SignalingWorker(int worker_id);
    ~SignalingWorker();

    bool Init();
    bool Start();
    void Stop();
    void Join();

    bool Notify(ssize_t msg);
    bool NotifyNewConnection(int fd);

    // timer
    void SetTimeOut(uint64_t usec);
    // event
    bool SendRtcMessage(const std::shared_ptr<RtcMessage> &msg);

  private:
    static void
    ConnectIOCall(EventLoop *el, IOWatcher *w, int fd, int events, void *data);
    static void ConnectTimeCall(EventLoop *el, TimerWatcher *w, void *data);

    static void WorkerRecvNotify(
        EventLoop *el, IOWatcher *w, int fd, int events, void *data);

  private:
    void HandleNotify(ssize_t msg);
    void StopEvent();
    void NewConnection(int fd);
    // event
    void ReadEvent(int fd);
    bool ProcessRequest(
        TcpConnection *conn, const rtc::Slice &head, const rtc::Slice &body);
    void ResponseServerOffer(std::shared_ptr<RtcMessage> msg);
    // net
    void CloseConnection(TcpConnection *conn);
    void RemoveConnection(TcpConnection *conn);

    // msg
    void ProcessRtcMsg();

    bool ProcessReadBuffer(TcpConnection *conn);
    void ProcessTimeout(TcpConnection *conn);
    bool ProcessPushRequest(
        TcpConnection *conn, const Json::Value &root, uint32_t log_id);
    // msg queue
    void PushMessage(const std::shared_ptr<RtcMessage> &msg);
    std::shared_ptr<RtcMessage> PopMessage();

  private:
    // option
    int _worker_id;
    unsigned long _timeout;

    EventLoop *_event_loop = nullptr;
    LockFreeQueue<int> _conn_queue;
    // communicate with main thread
    int _notify_recv_fd = -1;
    int _notify_send_fd = -1;
    std::unordered_map<int, TcpConnection *> _conn_pool;
    IOWatcher *_pipe_watcher = nullptr;
    // thread
    std::thread *_thread = nullptr;
    // msg
    std::queue<std::shared_ptr<RtcMessage>> _q_msg;
    std::mutex _q_msg_mtx;
};
} // namespace xrtc
