#pragma once
#include <string>
namespace xrtc {
struct GeneralConf {
    std::string log_dir;
    std::string log_name;
    std::string log_level;
    bool log_to_stderr;
};
int load_general_conf(const std::string &, GeneralConf *);
} // namespace xrtc