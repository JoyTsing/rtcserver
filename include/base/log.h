#pragma once
#include <string>
#include <rtc_base/logging.h>
namespace xrtc {
class Log : public rtc::LogSink {
  private:
    std::string _log_dir;
    std::string _log_name;
    std::string _log_level;
};
} // namespace xrtc