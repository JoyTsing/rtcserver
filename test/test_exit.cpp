#include "init/init.h"
#include <thread>

int main() {
    RTC_LOG(LS_INFO) << "==== In TEST ====";
    if (!xrtc::server_init_test()) { return 0; }

    auto *t = new std::thread([]() {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        xrtc::g_signaling_server->Stop();
        xrtc::g_rtc_server->Stop();
    });
    t->detach();
    xrtc::g_signaling_server->Join();
    xrtc::g_rtc_server->Join();
    return 0;
}