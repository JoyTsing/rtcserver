#pragma once
#include "base/conf.h"
#include "base/log.h"
#include "server/signaling_server.h"

namespace xrtc {

// TODO singleton
extern GeneralConf *g_conf;
extern Log *g_logger;
extern SignalingServer *g_signaling_server;

int init_general_conf(const std::string &conf_file);
int init_log(std::string log_dir, std::string log_name, std::string log_level);
int init_signaling_server();
} // namespace xrtc