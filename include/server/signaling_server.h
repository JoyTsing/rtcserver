#pragma once
#include <string>

namespace xrtc {

struct SignalingServerOptions {
    std::string host;
    int port;
    int worker_num;
    int connection_timeout;
};

class SignalingServer {
  public:
    SignalingServer();
    ~SignalingServer();
    int init(const std::string &conf_file);

  private:
    SignalingServerOptions _options;
};

} // namespace xrtc