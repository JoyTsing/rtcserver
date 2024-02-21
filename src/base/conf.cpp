#include "base/conf.h"
#include "yaml-cpp/exceptions.h"
#include <cstdio>
#include <yaml-cpp/yaml.h>
namespace xrtc {
int load_general_conf(const std::string &filename, GeneralConf *confs) {
    if (filename.empty() || confs == nullptr) {
        (void)fprintf(stderr, "filename or conf is nullptr\n");
        return -1;
    }
    confs->log_dir = "./log";
    confs->log_name = filename;
    confs->log_level = "info";
    confs->log_to_stderr = true;
    YAML::Node config = YAML::LoadFile(filename);
    try {
        confs->log_dir = config["log"]["log_dir"].as<std::string>();
        confs->log_name = config["log"]["log_name"].as<std::string>();
        confs->log_level = config["log"]["log_level"].as<std::string>();
        confs->log_to_stderr = config["log"]["log_to_stderr"].as<bool>();
    } catch (YAML::Exception &e) {
        (void)fprintf(
            stderr, "YAML::Exception line : %d,column: %d, error:%s\n",
            e.mark.line + 1, e.mark.column + 1, e.what());
        return -1;
    }

    return 0;
}
} // namespace xrtc