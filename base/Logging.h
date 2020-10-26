#ifndef TMUDUO_BASE_LOGGING_H_
#define TMUDUO_BASE_LOGGING_H_

#include <chrono>
#include "base/LogStream.h"
#include "base/TimeZone.h"
#include "base/Timestamp.h"

namespace tmuduo {

class Logger {
 public:
  //不同的日志级别
  enum LogLevel {
    TRACE = 0,
    DEBUG,
    INFO,
    WARN,
    ERROR,
    FATAL,
    NUM_LOG_LEVELS,
  };

  //实现存储日志的文件名
  class SourceFile {
   public:
    template <int N>
    SourceFile(const char (&arr)[N]) : data_(arr), size_(N - 1) {
      //获得程序的名字
      const char* slash = strrchr(data_, '/');
      if (slash) {
        data_ = slash + 1;
        size_ -= static_cast<int>(data_ - arr);
      }
    }
    explicit SourceFile(const char* filename) : data_(filename) {
      const char* slash = strrchr(filename, '/');
      if (slash) {
        data_ = slash + 1;
      }
      size_ = static_cast<int>(strlen(data_));
    }
    const char* data_;
    int size_;
  };
  Logger(SourceFile file, int line);
  Logger(SourceFile file, int line, LogLevel level);
  Logger(SourceFile file, int line, LogLevel level, const char* func);
  Logger(SourceFile file, int line, bool toAbort);
  ~Logger();
  void setTimeZone(const TimeZone& tz);
  LogStream& stream() { return impl_.stream_; }

  static LogLevel logLevel();
  static void setLogLevel(LogLevel level);
  typedef void (*OutputFunc)(const char* msg, int len);
  typedef void (*FlushFunc)();
  static void setOutput(OutputFunc);
  static void setFlush(FlushFunc);

 private:
  using time_point = std::chrono::high_resolution_clock::time_point;
  using high_resolution_clock = std::chrono::high_resolution_clock;
  using microseconds = std::chrono::microseconds;
  //此处与常见的 Implp 的做法有所不同。主要还是因为 Logger 利用了栈上空间
  //来处理流式日志风格的串话问题。而 Implp 的做法一般是使用指针指向一片动态
  //内存区域。
  class Impl {
   public:
    using LogLevel = Logger::LogLevel;
    Impl(LogLevel level, int old_errno, const SourceFile& file, int line);
    void formatTime();
    void finish();
    // time_point time_;
    Timestamp time_;
    LogStream stream_;
    LogLevel level_;
    int line_;
    SourceFile basename_;
  };
  Impl impl_;
};

extern Logger::LogLevel g_logLevel;

inline Logger::LogLevel Logger::logLevel() { return g_logLevel; }

//利用栈上的匿名对象来避免日志内容出现串话
#define LOG_TRACE                                                     \
  if (tmuduo::Logger::logLevel() <= tmuduo::Logger::TRACE)            \
  tmuduo::Logger(__FILE__, __LINE__, tmuduo::Logger::LogLevel::TRACE, \
                 __func__)                                            \
      .stream()
#define LOG_DEBUG                                                     \
  if (tmuduo::Logger::logLevel() <= tmuduo::Logger::DEBUG)            \
  tmuduo::Logger(__FILE__, __LINE__, tmuduo::Logger::LogLevel::DEBUG, \
                 __func__)                                            \
      .stream()
#define LOG_INFO                                                    \
  if (tmuduo::Logger::logLevel() <= tmuduo::Logger::LogLevel::INFO) \
  tmuduo::Logger(__FILE__, __LINE__).stream()
#define LOG_WARN \
  tmuduo::Logger(__FILE__, __LINE__, tmuduo::Logger::LogLevel::WARN).stream()
#define LOG_ERROR \
  tmuduo::Logger(__FILE__, __LINE__, tmuduo::Logger::LogLevel::ERROR).stream()
#define LOG_FATAL \
  tmuduo::Logger(__FILE__, __LINE__, tmuduo::Logger::LogLevel::FATAL).stream()
#define LOG_SYSERR tmuduo::Logger(__FILE__, __LINE__, false).stream()
#define LOG_SYSFATAL tmuduo::Logger(__FILE__, __LINE__, true).stream()

const char* strerror_tl(int savedErrno);

// Taken from glog/logging.h
//
// Check that the input is non NULL.  This very useful in constructor
// initializer lists.

// CHECK_NOTNULL 宏的作用与 assert 相同，但优点是不会受到 NDEBUG
// 模式的影响。即使在 Release 版本中也是有效的，有利于及早发现错误
#define CHECK_NOTNULL(val)                                                 \
  ::muduo::CheckNotNull(__FILE__, __LINE__, "'" #val "' Must be non NULL", \
                        (val))

// A small helper for CHECK_NOTNULL().
template <typename T>
T* CheckNotNull(Logger::SourceFile file, int line, const char* names, T* ptr) {
  if (ptr == NULL) {
    Logger(file, line, Logger::LogLevel::FATAL).stream() << names;
  }
  return ptr;
}

}  // namespace tmuduo

#endif  // TMUDUO_BASE_LOGGING_H_