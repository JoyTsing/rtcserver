#include "base/conf.h"
#include "base/log.h"
#include "init/init.h"
#include "rtc_base/logging.h"
#include <iostream>

int main() {
    int ret = xrtc::init_general_conf("./conf/general.yaml");
    if (ret != 0) { return -1; }
    ret = xrtc::init_log(
        xrtc::conf->log_dir, xrtc::conf->log_name, xrtc::conf->log_level);
    if (ret != 0) { return -1; }
    RTC_LOG(LS_VERBOSE) << "hello world";
    RTC_LOG(LS_WARNING) << "warning hello world";
    xrtc::logger->join();
    return 0;
}