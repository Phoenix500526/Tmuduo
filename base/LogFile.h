#ifndef TMUDUO_BASE_LOGFILE_H_
#define TMUDUO_BASE_LOGFILE_H_

#include "base/Mutex.h"
#include "base/TypeCast.h"

#include <memory>

namespace tmuduo {

namespace FileUtil {
// AppendFile 并不是线程安全的
class AppendFile;
}  // namespace FileUtil

class LogFile : noncopyable {
 public:
  LogFile(const std::string& basename, off_t rollSize, bool threadSafe = true,
          int flushInterval = 3, int checkEveryN = 1024);
  ~LogFile();
  void append(const char* logline, int len);
  void flush();
  bool rollFile();

 private:
  void append_unlocked(const char* logline, int len);
  static std::string getLogFileName(const std::string& basename, time_t* now);
  const std::string basename_;
  const off_t rollSize_;
  const int flushInterval_;
  const int checkEveryN_;
  int count_;
  std::unique_ptr<Mutex> mutex_;
  time_t startOfPeriod_;
  time_t lastRoll_;
  time_t lastFlush_;
  std::unique_ptr<FileUtil::AppendFile> file_;
  //每隔 24 小时完成一次文件滚动
  const static int kRollPerSeconds_ = 60 * 60 * 24;
};

}  // namespace tmuduo

#endif  // TMUDUO_BASE_LOGFILE_H_