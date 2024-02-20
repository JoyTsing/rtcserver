#pragma once
#include <rtc_base/logging.h>
#include <string>
namespace xrtc {
class Log : public rtc::LogSink {
  public:
    Log(const std::string &log_dir,
        const std::string &log_name,
        const std::string &log_level);
    ~Log() override;
    void OnLogMessage(
        const std::string &message, rtc::LoggingSeverity severity) override;
    void OnLogMessage(const std::string &message) override;

  private:
    std::string _log_dir;
    std::string _log_name;
    std::string _log_level;
};
} // namespace xrtc