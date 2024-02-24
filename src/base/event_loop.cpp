#include "base/event_loop.h"
#include <cstdint>
#include <libev/ev.h>
namespace xrtc {

#define TRANS_TO_EV_MASK(mask)                \
    (((mask) & EventLoop::READ ? EV_READ : 0) \
     | ((mask) & EventLoop::WRITE ? EV_WRITE : 0))

#define TRANS_FROM_EV_MASK(mask)              \
    (((mask) & EV_READ ? EventLoop::READ : 0) \
     | ((mask) & EV_WRITE ? EventLoop::WRITE : 0))

EventLoop::EventLoop(void *owner)
    : _loop(ev_loop_new(EVFLAG_AUTO)), owner(owner) {
}

EventLoop::~EventLoop() {
}

void EventLoop::Start() {
    ev_run(_loop);
}

void EventLoop::Stop() {
    ev_break(_loop, EVBREAK_ALL);
}

uint64_t EventLoop::Now() {
    return static_cast<uint64_t>(ev_now(_loop) * 1000000);
}

void EventLoop::generic_io_callback(
    struct ev_loop * /*loop*/, struct ev_io *w, int events) {
    auto *watcher = (IOWatcher *)(w->data);
    watcher->call(
        watcher->event_loop, watcher, w->fd, TRANS_FROM_EV_MASK(events),
        watcher->data); // 之前传入的是w->data，运行没有问题但是结束时候有问题
}

void EventLoop::generic_time_callback(
    struct ev_loop * /*loop*/, struct ev_timer *timer, int /*events*/) {
    auto *watcher = (TimeWatcher *)(timer->data);
    watcher->call(watcher->event_loop, watcher, watcher->data);
}

// io_event
EventLoop::IOWatcher *EventLoop::CreateIoEvent(io_callback_t call, void *data) {
    auto *w = new IOWatcher(this, call, data);
    ev_init(w->io, generic_io_callback);
    return w;
}

void EventLoop::StartIOEvent(IOWatcher *w, int fd, int mask) {
    struct ev_io *io = w->io;
    if (ev_is_active(io)) {
        int active_events = TRANS_FROM_EV_MASK(io->events);
        int new_events = active_events | mask;
        if (new_events == active_events) { return; }
        // add
        int event = TRANS_TO_EV_MASK(new_events);
        ev_io_stop(_loop, io);
        ev_io_set(io, fd, event);
        ev_io_start(_loop, io);
    }
    // new event
    int event = TRANS_TO_EV_MASK(mask);
    ev_io_set(io, fd, event);
    ev_io_start(_loop, io);
}

void EventLoop::StopIOEvent(IOWatcher *w, int fd, int mask) {
    struct ev_io *io = w->io;
    int active_events = TRANS_FROM_EV_MASK(io->events);
    int events = active_events & (~mask);

    if (events == active_events) { return; }
    events = TRANS_TO_EV_MASK(events);
    ev_io_stop(_loop, io);
    if (events != EV_NONE) {
        ev_io_set(io, fd, events);
        ev_io_start(_loop, io);
    }
}

void EventLoop::DeleteIOEvent(IOWatcher *w) {
    struct ev_io *io = w->io;
    ev_io_stop(_loop, io);
    delete w;
}

// time event
EventLoop::TimeWatcher *
EventLoop::CreateTimerEvent(time_callback_t call, void *data, bool repeate) {
    auto *watcher = new TimeWatcher(this, call, data, repeate);
    ev_init(watcher->timer, generic_time_callback);
    return watcher;
}

void EventLoop::StartTimerEvent(TimeWatcher *w, uint64_t usec) {
    struct ev_timer *timer = w->timer;
    float sec = float(usec) / 1000000;
    if (!w->repeate) {
        ev_timer_stop(_loop, timer);
        ev_timer_set(timer, sec, 0);
        ev_timer_start(_loop, timer);
        return;
    }
    timer->repeat = sec;
    ev_timer_again(_loop, timer);
}

void EventLoop::StopTimerEvent(TimeWatcher *w) {
    struct ev_timer *timer = w->timer;
    ev_timer_stop(_loop, timer);
}

void EventLoop::DeleteTimerEvent(TimeWatcher *w) {
    StopTimerEvent(w);
    delete w;
}

} // namespace xrtc