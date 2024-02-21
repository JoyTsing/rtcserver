#include "init/init.h"
#include "server/signaling_server.h"
#include <cstdio>
#include <string>
#include <unistd.h>
#include <utility>

namespace xrtc {

GeneralConf *g_conf = nullptr;
Log *g_logger = nullptr;
SignalingServer *g_signaling_server = nullptr;
// TODO  use std::once to ensure the function is called only once
int init_general_conf(const std::string &conf_file) {
    if (conf_file.empty() || g_conf != nullptr) {
        char *buffer = getcwd(nullptr, 0);
        (void)fprintf(stderr, "%s no conf_file\n", buffer);
        return -1;
    }
    g_conf = new ::xrtc::GeneralConf();
    int ret = xrtc::load_general_conf(conf_file, g_conf);
    if (ret != 0) {
        (void)fprintf(stderr, "load_general_conf failed\n");
        return -1;
    }
    return 0;
}

int init_log(std::string log_dir, std::string log_name, std::string log_level) {
    if (g_logger != nullptr) {
        (void)fprintf(stderr, "log already initialized\n");
        return -1;
    }
    g_logger = new xrtc::Log(
        std::move(log_dir), std::move(log_name), std::move(log_level));
    int ret = g_logger->init();
    if (ret != 0) {
        (void)fprintf(stderr, "log init failed\n");
        return -1;
    }
    // start log thread
    g_logger->start();
    return 0;
}

int init_signaling_server(const std::string &conf_file) {
    g_signaling_server = new SignalingServer();
    return g_signaling_server->init(conf_file);
}
} // namespace xrtc