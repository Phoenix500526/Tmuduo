#include <chrono>
#include <thread>

#include "base/CurrentThread.h"

namespace tmuduo {
void CurrentThread::cacheTid() {
  if (0 == t_cachedTid) {
    auto id = std::this_thread::get_id();
    t_cachedTid = std::hash<std::thread::id>()(id);
    t_tidStringLength =
        snprintf(t_tidString, sizeof t_tidString, "%5lu ", t_cachedTid);
  }
}

bool CurrentThread::isMainThread() {
  auto id = std::this_thread::get_id();
  return std::hash<std::thread::id>()(id) == tid();
}

void CurrentThread::sleepUsec(int64_t usec) {
  static const int kMicrosecondsPerSecond = 1000 * 1000;
  struct timespec ts = {0, 0};
  ts.tv_sec = static_cast<time_t>(usec / kMicrosecondsPerSecond);
  ts.tv_nsec = static_cast<long>((usec % kMicrosecondsPerSecond) * 1000);
  ::nanosleep(&ts, NULL);
}

}  // namespace tmuduo