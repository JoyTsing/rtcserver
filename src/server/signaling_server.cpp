#include "server/signaling_server.h"
#include "net/socket.h"
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
    // config
    YAML::Node config = YAML::LoadFile(conf_file);
    try {
        _options.host = config["server"]["host"].as<std::string>();
        _options.port = config["server"]["port"].as<int>();
        _options.worker_num = config["server"]["worker_num"].as<int>();
        _options.connection_timeout =
            config["server"]["connection_timeout"].as<int>();
    } catch (YAML::Exception &e) {
        RTC_LOG(LS_WARNING)
            << "catch a YAML exception in signaling, message:" << e.msg;
        return -1;
    }
    // net
    _listen_fd = CreateTcpServer(_options.host, _options.port);
    //
    return 0;
}

} // namespace xrtc
