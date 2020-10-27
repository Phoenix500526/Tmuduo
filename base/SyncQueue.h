#ifndef TMUDUO_BASE_SYNCQUEUE_H_
#define TMUDUO_BASE_SYNCQUEUE_H_

#include "base/Condition.h"
#include "base/Mutex.h"
#include "base/Thread.h"

#include <list>
namespace tmuduo {
template <typename T>
class SyncQueue : noncopyable {
 public:
  SyncQueue(int maxsize)
      : mutex_(), full_(), empty_(), maxSize_(maxsize), running_(true) {}
  ~SyncQueue() = default;
  void put(const T& x) { add(x); }
  void put(T&& x) { add(std::forward<T>(x)); }
  void take(T& t) {
    UniqueLock lock(mutex_);
    while (queue_.empty() && running_) {
      empty_.wait(lock);
    }
    if (!running_) return;
    t = queue_.front();
    queue_.pop_front();
    full_.notify_one();
  }

  size_t size() const {
    UniqueLock lock(mutex_);
    return queue_.size();
  }

  void stop() NO_THREAD_SAFETY_ANALYSIS {
    {
      UniqueLock lock(mutex_);
      running_ = false;
    }
    full_.notify_all();
    empty_.notify_all();
  }

 private:
  template <typename F>
  void add(F&& x) {
    UniqueLock lock(mutex_);
    while (queue_.size() >= maxSize_ && running_) {
      full_.wait(lock);
    }
    if (!running_) return;
    queue_.push_back(std::forward<F>(x));
    empty_.notify_one();
  }

 private:
  std::list<T> queue_;
  mutable Mutex mutex_;
  Condition full_ GUARDED_BY(mutex_);   //同步队列已满
  Condition empty_ GUARDED_BY(mutex_);  //同步队列已空
  int maxSize_;
  bool running_;
};
}  // namespace tmuduo

#endif  // TMUDUO_BASE_SYNCQUEUE_H_