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
RtcServer *g_rtc_server = nullptr;

// TODO  use std::once to ensure the function is called only once
bool init_general_conf(const std::string &conf_file) {
    if (conf_file.empty() || g_conf != nullptr) {
        char *buffer = getcwd(nullptr, 0);
        (void)fprintf(stderr, "%s no conf_file\n", buffer);
        return false;
    }
    g_conf = new ::xrtc::GeneralConf();
    int ret = xrtc::load_general_conf(conf_file, g_conf);
    if (ret != 0) {
        (void)fprintf(stderr, "load_general_conf failed\n");
        return false;
    }
    return true;
}

bool init_log(
    std::string log_dir, std::string log_name, std::string log_level) {
    if (g_logger != nullptr) {
        (void)fprintf(stderr, "log already initialized\n");
        return false;
    }
    g_logger = new xrtc::Log(
        std::move(log_dir), std::move(log_name), std::move(log_level));
    int ret = g_logger->init();
    if (ret != 0) {
        (void)fprintf(stderr, "log init failed\n");
        return false;
    }
    // start log thread
    g_logger->start();
    return true;
}

bool init_signaling_server(const std::string &conf_file) {
    g_signaling_server = new SignalingServer();
    return g_signaling_server->Init(conf_file);
}

bool init_rtc_server(const std::string &conf_file) {
    g_rtc_server = new RtcServer();
    return g_rtc_server->Init(conf_file);
}

// init all
bool server_init() {
    if (!xrtc::init_general_conf("./conf/general.yaml")) { return false; }

    if (!xrtc::init_log(
            xrtc::g_conf->log_dir, xrtc::g_conf->log_name,
            xrtc::g_conf->log_level)) {
        return false;
    }

    if (!xrtc::init_signaling_server("./conf/signaling_server.yaml")) {
        return false;
    }

    if (!xrtc::init_rtc_server("./conf/rtc_server.yaml")) { return false; }

    xrtc::g_signaling_server->Start();
    xrtc::g_rtc_server->Start();
    return true;
}

bool server_init_test() {
    if (!xrtc::init_general_conf("../../conf/general.yaml")) { return false; }

    if (!xrtc::init_log(
            xrtc::g_conf->log_dir, xrtc::g_conf->log_name,
            xrtc::g_conf->log_level)) {
        return false;
    }
    if (!xrtc::init_signaling_server("../../conf/signaling_server.yaml")) {
        return false;
    }

    if (!xrtc::init_rtc_server("../../conf/rtc_server.yaml")) { return false; }

    xrtc::g_signaling_server->Start();
    xrtc::g_rtc_server->Start();
    return true;
}
} // namespace xrtc