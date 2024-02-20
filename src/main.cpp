#include "base/conf.h"
#include <cstdio>

xrtc::GeneralConf *conf = nullptr;

int init_general_conf(const std::string &conf_file) {
    if (conf_file.empty() || conf == nullptr) {
        (void)fprintf(stderr, "conf_file or conf is nullptr\n");
        return -1;
    }
    conf = new ::xrtc::GeneralConf();
    int ret = xrtc::load_general_conf(conf_file, conf);
    if (ret != 0) { return -1; }
    return 0;
}

int main() {
    int ret = init_general_conf("./conf/general.yaml");
    if (ret != 0) { return -1; }
    return 0;
}