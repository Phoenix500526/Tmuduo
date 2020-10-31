#ifndef TMUDUO_NET_EVENTLOOPTHREADPOOL_H_
#define TMUDUO_NET_EVENTLOOPTHREADPOOL_H_

#include "base/TypeCast.h"
#include "base/noncopyable.h"

#include <functional>
#include <vector>

namespace tmuduo {
namespace net {

class EventLoop;
class EventLoopThread;

class EventLoopThreadPool : noncopyable {
 public:
  using ThreadInitCallback = std::function<void(EventLoop*)>;
  EventLoopThreadPool(EventLoop* baseLoop, const std::string& name);
  ~EventLoopThreadPool();
  void setThreadNum(int numThreads) { numThreads_ = numThreads; }
  void start(const ThreadInitCallback& cb = ThreadInitCallback());

  //使用 round-robin 算法,在 start() 调用后生效
  EventLoop* getNextLoop();

  //同一个 hash 值会得到同一个 EventLoop 对象
  EventLoop* getLoopForHash(size_t hashCode);

  std::vector<EventLoop*> getAllLoops();

  bool started() const { return started_; }

  const std::string& name() const { return name_; }

 private:
  EventLoop* baseLoop_;
  std::string name_;
  bool started_;
  int numThreads_;
  int next_;
  std::vector<std::unique_ptr<EventLoopThread>> threads_;
  std::vector<EventLoop*> loops_;
};

}  // namespace net
}  // namespace tmuduo

#endif  // TMUDUO_NET_EVENTLOOPTHREADPOOL_H_