#include "base/Logging.h"
#include "base/CurrentThread.h"

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sstream>
#include <string>

namespace tmuduo {

thread_local char t_errnobuf[512];
thread_local char t_time[64];
thread_local time_t t_lastSecond;

const char* strerror_tl(int saveErrno) {
  return strerror_r(saveErrno, t_errnobuf, sizeof t_errnobuf);
}

Logger::LogLevel initLogLevel() {
  if (::getenv("TMUDUO_LOG_TRACE"))
    return Logger::LogLevel::TRACE;
  else if (::getenv("TMUDUO_LOG_DEBUG"))
    return Logger::LogLevel::DEBUG;
  else
    return Logger::LogLevel::INFO;
}

Logger::LogLevel g_logLevel = initLogLevel();

const char* LogLevelName[Logger::LogLevel::NUM_LOG_LEVELS] = {
    "TRACE", "DEBUG", "INFO", "WARN", "ERROR", "FATAL",
};

class T {
 public:
  T(const char* str, unsigned len) : str_(str), len_(len) {
    assert(strlen(str) == len);
  }
  const char* str_;
  const unsigned len_;
};

inline LogStream& operator<<(LogStream& s, T v) {
  s.append(v.str_, v.len_);
  return s;
}

inline LogStream& operator<<(LogStream& s, const Logger::SourceFile& v) {
  s.append(v.data_, v.size_);
  return s;
}

void defaultOutput(const char* msg, int len) {
  size_t n = fwrite(msg, 1, len, stdout);
  assert(n == len);
  (void)n;
}

void defaultFlush() { fflush(stdout); }

Logger::OutputFunc g_output = defaultOutput;
Logger::FlushFunc g_flush = defaultFlush;

void Logger::Impl::finish() {
  stream_ << " - " << basename_ << ':' << line_ << '\n';
}

Logger::Logger(SourceFile file, int line)
    : impl_(LogLevel::INFO, 0, file, line) {}

Logger::Logger(SourceFile file, int line, LogLevel level, const char* func)
    : impl_(level, 0, file, line) {
  impl_.stream_ << func << ' ';
}

Logger::Logger(SourceFile file, int line, LogLevel level)
    : impl_(level, 0, file, line) {}

Logger::Logger(SourceFile file, int line, bool toAbort)
    : impl_(toAbort ? LogLevel::FATAL : LogLevel::ERROR, errno, file, line) {}

Logger::~Logger() {
  impl_.finish();
  const LogStream::Buffer& buf(stream().buffer());
  g_output(buf.data(), buf.length());
  if (impl_.level_ == LogLevel::FATAL) {
    g_flush();
    abort();
  }
}

void Logger::setLogLevel(Logger::LogLevel level) { g_logLevel = level; }

void Logger::setOutput(OutputFunc out) { g_output = out; }

void Logger::setFlush(FlushFunc flush) { g_flush = flush; }

Logger::Impl::Impl(LogLevel level, int saveErrno, const SourceFile& file,
                   int line)
    : time_(high_resolution_clock::now()),
      stream_(),
      level_(level),
      line_(line),
      basename_(file) {
  formatTime();
  stream_ << CurrentThread::tid() << " " << T(LogLevelName[level], 6);
  if (saveErrno != 0) {
    stream_ << strerror_tl(saveErrno) << "(errno = " << saveErrno << ") ";
  }
}

void Logger::Impl::formatTime() {
  static const int kMicroSecondsPerSecond = 1000 * 1000;
  microseconds u_sec =
      std::chrono::duration_cast<microseconds>(time_.time_since_epoch());
  time_t seconds =
      std::chrono::duration_cast<std::chrono::seconds>(u_sec).count();
  if (seconds != t_lastSecond) {
    t_lastSecond = seconds;
    struct tm tm_time;
    localtime_r(&seconds, &tm_time);
    int len =
        snprintf(t_time, sizeof t_time, "%04d%02d%02d %02d:%02d:%02d",
                 tm_time.tm_year + 1900, tm_time.tm_mon + 1, tm_time.tm_mday,
                 tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec);
    assert(len == 17);
    (void)len;
  }
  Fmt us(".%06d ", u_sec.count() % kMicroSecondsPerSecond);
  assert(8 == us.length());
  stream_ << T(t_time, 17) << T(us.data(), 8);
}

}  // namespace tmuduo
