#ifndef TMUDUO_BASE_THREADPOOL_H_
#define TMUDUO_BASE_THREADPOOL_H_

#include "base/Condition.h"
#include "base/Mutex.h"
#include "base/SyncQueue.h"
#include "base/Thread.h"

#include <atomic>
#include <vector>

namespace tmuduo {

class ThreadPool : noncopyable {
 public:
  using Task = std::function<void()>;
  ThreadPool(int maxSize, const std::string& name = std::string("ThreadPool"));
  ~ThreadPool();

  void setThreadInitCallback(const Task& cb) { threadInitCallback_ = cb; }
  const std::string& name() const { return name_; }

  size_t queueSize() const;  //不能是内联的
  void start(int numThreads);
  void stop();
  void run(Task&& f);

 private:
  void runInThread();
  std::string name_;
  Task threadInitCallback_;
  std::vector<std::unique_ptr<Thread>> threads_;
  SyncQueue<Task> queue_;
  std::atomic<bool> running_;
};

}  // namespace tmuduo
#endif  // TMUDUO_BASE_THREADPOOL_H_