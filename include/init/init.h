#pragma once
#include "base/conf.h"
#include "base/log.h"
#include "server/rtc_server.h"
#include "server/signaling_server.h"

namespace xrtc {

bool server_init();
bool server_init_test();

// TODO singleton
extern GeneralConf *g_conf;
extern Log *g_logger;
extern SignalingServer *g_signaling_server;
extern RtcServer *g_rtc_server;

bool init_general_conf(const std::string &conf_file);
bool init_log(std::string log_dir, std::string log_name, std::string log_level);
bool init_signaling_server(const std::string &conf_file);
bool init_rtc_server(const std::string &conf_file);
} // namespace xrtc