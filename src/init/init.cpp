#include "init/init.h"
#include <cstdio>
#include <string>
#include <unistd.h>
#include <utility>

namespace xrtc {

GeneralConf *conf = nullptr;
Log *logger = nullptr;

int init_general_conf(const std::string &conf_file) {
    if (conf_file.empty() || conf != nullptr) {
        char *buffer = getcwd(nullptr, 0);
        (void)fprintf(stderr, "%s no conf_file\n", buffer);
        return -1;
    }
    conf = new ::xrtc::GeneralConf();
    int ret = xrtc::load_general_conf(conf_file, conf);
    if (ret != 0) {
        (void)fprintf(stderr, "load_general_conf failed\n");
        return -1;
    }
    return 0;
}

int init_log(std::string log_dir, std::string log_name, std::string log_level) {
    if (logger != nullptr) {
        (void)fprintf(stderr, "log already initialized\n");
        return -1;
    }
    logger = new xrtc::Log(
        std::move(log_dir), std::move(log_name), std::move(log_level));
    int ret = logger->init();
    if (ret != 0) {
        (void)fprintf(stderr, "log init failed\n");
        return -1;
    }
    // start log thread
    logger->start();
    return 0;
}
} // namespace xrtc