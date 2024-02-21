#include "init/init.h"

bool server_init() {
    if (xrtc::init_general_conf("./conf/general.yaml") != 0) { return false; }

    if (xrtc::init_log(
            xrtc::g_conf->log_dir, xrtc::g_conf->log_name,
            xrtc::g_conf->log_level)
        != 0) {
        return false;
    }
    if (xrtc::init_signaling_server("./conf/signaling_server.yaml") != 0) {
        return false;
    }

    return true;
}

int main() {
    if (!server_init()) { return -1; }
    xrtc::g_logger->join();
    return 0;
}