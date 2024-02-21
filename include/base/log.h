#pragma once
#include <fstream>
#include <mutex>
#include <queue>
#include <rtc_base/logging.h>
#include <string>
#include <thread>
namespace xrtc {
rtc::LoggingSeverity GetLogSeverity(const std::string &level);
class Log : public rtc::LogSink {
  public:
    Log(std::string log_dir, std::string log_name, std::string log_level);
    int init();
    ~Log() override;
    void OnLogMessage(
        const std::string &message, rtc::LoggingSeverity severity) override;
    void OnLogMessage(const std::string &message) override;

    static void SetLogToStderr(bool on);

    // log thread
    bool start(int ms_time = 50);
    void stop();
    void join();

  private:
    // log thread
    std::queue<std::string> _log_queue;
    std::queue<std::string> _log_warn_queue;

    std::mutex _mtx;
    std::mutex _mtx_warn;
    std::thread *_thread = nullptr;
    std::atomic<bool> _running{false};

  private:
    // log file
    std::string _log_dir;
    std::string _log_name;
    std::string _log_level;
    std::string _log_file;
    std::string _log_file_warn;

    std::ofstream _log_stream;
    std::ofstream _log_stream_warn;

  private:
    void LogToStream(std::ofstream &);
    void LogToStreamWarn(std::ofstream &);
};
} // namespace xrtc