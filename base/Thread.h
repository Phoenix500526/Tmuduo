#ifndef TMUDUO_BASE_THREAD_H_
#define TMUDUO_BASE_THREAD_H_
#include "base/CurrentThread.h"
#include "base/noncopyable.h"

#include <functional>
#include <thread>

namespace tmuduo {

class Thread : noncopyable {
 public:
  using ThreadFunc = std::function<void()>;
  Thread(ThreadFunc func, const std::string& name = std::string());
  ~Thread();
  Thread(Thread&& rhs) noexcept;
  Thread& operator=(Thread&& rhs) noexcept;
  void join();

 private:
  //线程实体
  std::thread thread_;
  void runInThread(ThreadFunc func, const std::string& name);
};

}  // namespace tmuduo
#endif  // TMUDUO_BASE_THREAD_H_