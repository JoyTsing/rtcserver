#include "init/init.h"

int main() {
    RTC_LOG(LS_INFO) << "==== In MAIN ====";
    if (!xrtc::server_init()) { return -1; }
    xrtc::g_signaling_server->Join();
    return 0;
}
