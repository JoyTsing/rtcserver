#include "base/log.h"

namespace xrtc {
Log::Log(
    const std::string &log_dir,
    const std::string &log_name,
    const std::string &log_level)
    : _log_dir(log_dir), _log_name(log_name), _log_level(log_level) {
}
Log::~Log() {
}
void Log::OnLogMessage(
    const std::string &message, rtc::LoggingSeverity severity) {
}

void Log::OnLogMessage(const std::string & /*message*/) {
    throw std::runtime_error("OnLogMessage not implemented");
}

} // namespace xrtc