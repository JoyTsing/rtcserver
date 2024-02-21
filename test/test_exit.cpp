#include "init/init.h"
#include <thread>

void test_exit() {
    if (!xrtc::server_init_test()) { return; }

    auto func = [](int sig) -> void {
        RTC_LOG(LS_INFO) << "receive signal " << sig
                         << ", stop signaling server";
        if (sig == SIGINT || sig == SIGTERM) {
            if (xrtc::g_signaling_server) { xrtc::g_signaling_server->Stop(); }
        }
    };
    (void)signal(SIGINT, func);
    (void)signal(SIGTERM, func);
    auto *t = new std::thread([]() {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        xrtc::g_signaling_server->Stop();
    });
    t->detach();
    xrtc::g_signaling_server->Join();
}

int main() {
    RTC_LOG(LS_INFO) << "==== In TEST ====";
    test_exit();
    return 0;
}