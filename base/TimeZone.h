#ifndef TMUDUO_BASE_TIMEZONE_H_
#define TMUDUO_BASE_TIMEZONE_H_

#include <time.h>
#include <memory>
#include "base/copyable.h"

namespace tmuduo {

class TimeZone : copyable {
 public:
  explicit TimeZone(const char* zonefile);
  TimeZone(int eastOfUtc, const char* tzname);
  TimeZone() = default;
  ~TimeZone() = default;
  //显示调用 data_ 的 bool 转换函数
  bool valid() const { return static_cast<bool>(data_); }

  struct tm toLocalTime(time_t secondsSinceEpoch) const;
  time_t fromLocalTime(const struct tm&) const;

  static struct tm toUtcTime(time_t secondsSinceEpoch, bool yday = false);

  static time_t fromUtcTime(const struct tm&);

  static time_t fromUtcTime(int year, int month, int day, int hour, int minute,
                            int seconds);
  struct Data;

 private:
  std::shared_ptr<Data> data_;
};

}  // namespace tmuduo

#endif  // TMUDUO_BASE_TIMEZONE_H_