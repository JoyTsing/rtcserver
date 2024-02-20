#include "base/conf.h"
#include <cstdio>
#include <yaml-cpp/yaml.h>
namespace xrtc {
int load_general_conf(const std::string &filename, GeneralConf *conf) {
    if (filename.empty() || conf == nullptr) {
        (void)fprintf(stderr, "filename or conf is nullptr\n");
        return -1;
    }
    conf->log_dir = "./log";
    conf->log_name = filename;
    conf->log_level = "info";
    conf->log_to_stderr = true;

    YAML::Node node = YAML::LoadFile(filename);
    return 0;
}
} // namespace xrtc