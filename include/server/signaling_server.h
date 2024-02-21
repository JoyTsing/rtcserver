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

    int _listen_fd = -1;
};

} // namespace xrtc