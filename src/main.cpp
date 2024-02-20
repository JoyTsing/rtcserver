#include "base/conf.h"
#include <cstdio>
#include <unistd.h>
xrtc::GeneralConf *conf = nullptr;

int init_general_conf(const std::string &conf_file) {
    if (conf_file.empty() || conf != nullptr) {
        char *buffer = getcwd(NULL, 0);
        (void)fprintf(stderr, "%s no conf_file\n", buffer);
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