#pragma once
#include "base/lock_free_queue.h"
#include "server/rtc_server.h"

namespace xrtc {
class RtcWorker {
  public:
    using IOWatcher = EventLoop::IOWatcher;
    enum { QUIT = 0, RTC_MSG = 1 };
    RtcWorker(int worker_id, const RtcServerOptions &options);
    ~RtcWorker();

    bool Init();
    bool Start();
    void Stop();

    void Join();

    bool Notify(ssize_t msg);

    bool SendRtcMessage(std::shared_ptr<RtcMessage> msg);

  private:
    static void WorkerRecvNotify(
        EventLoop *el, IOWatcher *w, int fd, int events, void *data);

  private:
    // event
    void ProcessPush(std::shared_ptr<RtcMessage> msg);
    void StopEvent();
    void HandleNotify(int msg);
    void ProcessRtcMsg();
    void PushMessage(const std::shared_ptr<RtcMessage> &msg);
    bool PopMessage(std::shared_ptr<RtcMessage> &msg);

  private:
    EventLoop *_event_loop;
    RtcServerOptions _options;
    int _worker_id;

    //
    IOWatcher *_pipe_watcher = nullptr;
    int _notify_recv_fd = -1;
    int _notify_send_fd = -1;

    LockFreeQueue<std::shared_ptr<RtcMessage>> _q_msg;
    // thread
    std::thread *_thread = nullptr;
};
} // namespace xrtc