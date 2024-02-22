#include "init/init.h"

void bind_signal() {
    auto func = [](int sig) -> void {
        RTC_LOG(LS_INFO) << "receive signal " << sig
                         << ", stop signaling server";
        if (sig == SIGINT || sig == SIGTERM) {
            if (xrtc::g_signaling_server) { xrtc::g_signaling_server->Stop(); }
        }
    };
    (void)signal(SIGINT, func);
    (void)signal(SIGTERM, func);
}

int main() {
    RTC_LOG(LS_INFO) << "==== In MAIN ====";
    if (!xrtc::server_init()) { return -1; }
    bind_signal();
    xrtc::g_signaling_server->Join();
    return 0;
}
