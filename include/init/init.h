#pragma once
#include "base/conf.h"
#include "base/log.h"

namespace xrtc {

// TODO singleton
extern GeneralConf *conf;
extern Log *logger;

int init_general_conf(const std::string &conf_file);
int init_log(std::string log_dir, std::string log_name, std::string log_level);
} // namespace xrtc