#include "base/log.h"
#include "rtc_base/logging.h"
#include <cerrno>
#include <chrono>
#include <cstdio>
#include <iostream>
#include <mutex>
#include <sstream>
#include <sys/stat.h>
#include <thread>

namespace xrtc {

Log::Log(std::string log_dir, std::string log_name, std::string log_level)
    : _log_dir(std::move(log_dir))
    , _log_name(std::move(log_name))
    , _log_level(std::move(log_level))
    , _log_file(_log_dir + "/" + _log_name + ".log")
    , _log_file_warn(_log_dir + "/" + _log_name + ".log.warn") {
}

Log::~Log() {
    _log_stream.close();
    _log_stream_warn.close();
    stop();
}

int Log::init() {
    // set log
    rtc::LogMessage::ConfigureLogging("thread tstamp");
    rtc::LogMessage::SetLogPathPrefix("/rtcserver");
    rtc::LogMessage::AddLogToStream(this, GetLogSeverity(_log_level));
    // file
    int ret = mkdir(_log_dir.c_str(), 0755);
    if (ret != 0 && errno != EEXIST) {
        (void)fprintf(stderr, "create log_dir[%s] faild\n", _log_dir.c_str());
    }
    // open file
    _log_stream.open(_log_file, std::ios::app);
    if (!_log_stream.is_open()) {
        (void)fprintf(stderr, "open log_file[%s] faild\n", _log_file.c_str());
        return -1;
    }
    _log_stream_warn.open(_log_file_warn, std::ios::app);
    if (!_log_stream_warn.is_open()) {
        (void)fprintf(
            stderr, "open log_file_warn[%s] faild\n", _log_file_warn.c_str());
        return -1;
    }
    return 0;
}

// auto call function when log message
void Log::OnLogMessage(
    const std::string &message, rtc::LoggingSeverity severity) {
    if (severity >= rtc::LS_WARNING) {
        std::unique_lock<std::mutex> lock(_mtx_warn);
        _log_warn_queue.push(message);
    } else {
        std::unique_lock<std::mutex> lock(_mtx);
        _log_queue.push(message);
    }
}

void Log::OnLogMessage(const std::string & /*message*/) {
    throw std::runtime_error("OnLogMessage not implemented");
}

void Log::SetLogToStderr(bool on) {
    rtc::LogMessage::SetLogToStderr(on);
}

rtc::LoggingSeverity GetLogSeverity(const std::string &level) {
    if ("verbose" == level) { return rtc::LS_VERBOSE; }
    if ("info" == level) { return rtc::LS_INFO; }
    if ("warning" == level) { return rtc::LS_WARNING; }
    if ("error" == level) { return rtc::LS_ERROR; }

    return rtc::LS_NONE;
}

bool Log::start(int ms_time) {
    if (_running) {
        (void)fprintf(stderr, "log thread already running\n");
        return false;
    }
    _running = true;
    _thread = new std::thread([=]() {
        struct stat stat_data;

        while (_running) {
            // check log file
            if (stat(_log_file.c_str(), &stat_data) < 0) {
                _log_stream.close();
                _log_stream.open(_log_file, std::ios::app);
            }
            if (stat(_log_file_warn.c_str(), &stat_data) < 0) {
                _log_stream_warn.close();
                _log_stream_warn.open(_log_file_warn, std::ios::app);
            }

            // write log
            LogToStream(_log_stream);
            // write warn log
            LogToStreamWarn(_log_stream_warn);
            std::this_thread::sleep_for(
                std::chrono::milliseconds(ms_time)); // 50ms flush
        }
    });
    return true;
}

void Log::stop() {
    _running = false;
    if (_thread != nullptr) {
        if (_thread->joinable()) { _thread->join(); }
        delete _thread;
        _thread = nullptr;
    }
}

void Log::join() {
    if (_thread != nullptr && _thread->joinable()) { _thread->join(); }
}

void Log::LogToStream(std::ofstream &stream) {
    std::stringstream ss;

    {
        std::unique_lock<std::mutex> lock(_mtx);
        if (_log_queue.empty()) { return; }
        while (!_log_queue.empty()) {
            ss << _log_queue.front();
            _log_queue.pop();
        }
    }
    stream << ss.str();
    stream.flush();
}

void Log::LogToStreamWarn(std::ofstream &stream) {
    std::stringstream ss;

    {
        std::unique_lock<std::mutex> lock(_mtx);
        if (_log_warn_queue.empty()) { return; }
        while (!_log_warn_queue.empty()) {
            ss << _log_warn_queue.front();
            _log_warn_queue.pop();
        }
    }
    stream << ss.str();
    stream.flush();
}
} // namespace xrtc