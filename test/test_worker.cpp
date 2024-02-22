#include "init/init.h"
#include <thread>

void test_worker() {
    if (!xrtc::server_init_test()) { return; }

    auto *t = new std::thread([]() {
        std::this_thread::sleep_for(std::chrono::seconds(3));
        xrtc::g_signaling_server->Stop();
    });
    t->detach();
}

int main() {
    RTC_LOG(LS_INFO) << "==== In TEST ====";
    test_worker();
    xrtc::g_signaling_server->Join();
    return 0;
}