#include "base/event_loop.h"
#include "libev/ev.h"
namespace xrtc {

EventLoop::EventLoop(void *owner) : owner(owner) {
    _loop = ev_loop_new(EVFLAG_AUTO);
}

EventLoop::~EventLoop() {
    ev_loop_destroy(_loop);
}

void EventLoop::start() {
    ev_run(_loop);
}

void EventLoop::stop() {
    ev_break(_loop, EVBREAK_ALL);
}

EventLoop::IOWatcher *EventLoop::CreateIoEvent(io_callback_t call, void *data) {
}
} // namespace xrtc