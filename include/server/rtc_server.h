#pragma once

#include "base/event_loop.h"
#include "net/server_def.h"
#include <memory>
#include <mutex>
#include <queue>
#include <string>
#include <thread>
#include <unistd.h>
#include <vector>
namespace xrtc {
struct RtcServerOptions {
    int worker_num;
};

class RtcWorker;
class RtcServer {
  public:
    enum {
        QUIT = 0,
        RTC_MSG = 1,
    };
    using IOWatcher = EventLoop::IOWatcher;

  public:
    RtcServer();
    ~RtcServer();

    bool Init(const std::string &file);
    bool Start();
    void Stop();
    void Join();

    bool Notify(ssize_t msg);
    bool SendRtcMessage(std::shared_ptr<RtcMessage> msg);

  private:
    static void ServerRecvNotify(
        EventLoop *el, IOWatcher *w, int fd, int events, void *data);

  private:
    void PushMessage(const std::shared_ptr<RtcMessage> &msg);
    std::shared_ptr<RtcMessage> PopMessage();
    void StopEvent();
    void HandleNotify(ssize_t msg);
    void ProcessRtcMsg();

    bool CreateWorker(int worker_id);
    RtcWorker *GetWorker(const std::string &stream_name);

  private:
    EventLoop *_event_loop;
    RtcServerOptions _options;

    IOWatcher *_pipe_watcher = nullptr;

    int _notify_recv_fd = -1;
    int _notify_send_fd = -1;
    // thread
    std::thread *_thread = nullptr;
    // worker pool
    std::vector<RtcWorker *> _workers;
    // message queue
    std::mutex _q_mtx;
    std::queue<std::shared_ptr<RtcMessage>> _queue_msg;
};

} // namespace xrtc