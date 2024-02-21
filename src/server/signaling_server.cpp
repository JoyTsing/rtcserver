#include "server/signaling_server.h"
#include "rtc_base/logging.h"
#include <yaml-cpp/yaml.h>

namespace xrtc {

SignalingServer::SignalingServer() {
}

SignalingServer::~SignalingServer() {
}

int SignalingServer::init(const std::string &conf_file) {
    if (conf_file.empty()) {
        RTC_LOG(LS_WARNING) << "signaling server conf file is empty";
        return -1;
    }
    try {
        YAML::Node config = YAML::LoadFile(conf_file);

        _options.host = config["server"]["host"].as<std::string>();
        _options.port = config["server"]["port"].as<int>();
        _options.worker_num = config["server"]["worker_num"].as<int>();
        _options.connection_timeout =
            config["server"]["connection_timeout"].as<int>();
    } catch (YAML::Exception &e) {
        RTC_LOG(LS_WARNING)
            << "catch a YAML exception,line:" << e.mark.line + 1
            << ", column:" << e.mark.column + 1 << ", message:" << e.msg;
        return -1;
    }
    return 0;
}

} // namespace xrtc
