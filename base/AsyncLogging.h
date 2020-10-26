#ifndef TMUDUO_BASE_ASYNCLOGGING_H_
#define TMUDUO_BASE_ASYNCLOGGING_H_

#include "base/Condition.h"
#include "base/LogStream.h"
#include "base/Mutex.h"
#include "base/Thread.h"

#include <atomic>
#include <vector>

namespace tmuduo {

class AsyncLogging : noncopyable {
 public:
  AsyncLogging(const std::string& basename, off_t rollSize,
               int flushInterval = 3);
  ~AsyncLogging() {
    if (running_) {
      stop();
    }
  }
  void append(const char* logline, int len);
  void stop() NO_THREAD_SAFETY_ANALYSIS {
    running_ = false;
    cond_.notify_one();
    thread_.join();
  }

 private:
  using Buffer = detail::FixedBuffer<detail::kLargeBuffer>;
  using BufferVector = std::vector<std::unique_ptr<Buffer>>;
  using BufferPtr = BufferVector::value_type;
  using Seconds = std::chrono::seconds;
  void threadFunc();
  const Seconds flushInterval_;
  std::atomic<bool> running_;
  const std::string basename_;
  const off_t rollSize_;
  Thread thread_;
  Mutex mutex_;
  Condition cond_ GUARDED_BY(mutex_);
  BufferPtr currentBuffer_ GUARDED_BY(mutex_);
  BufferPtr nextBuffer_ GUARDED_BY(mutex_);
  BufferVector buffers_ GUARDED_BY(mutex_);
};

}  // namespace tmuduo

#endif  // TMUDUO_BASE_ASYNCLOGGING_H_