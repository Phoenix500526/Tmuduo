#ifndef TMUDUO_NET_EVENTLOOPTHREAD_H_
#define TMUDUO_NET_EVENTLOOPTHREAD_H_

#include "base/CountDownLatch.h"
#include "base/Mutex.h"
#include "base/Thread.h"

namespace tmuduo {
namespace net {

class EventLoop;
class EventLoopThread : noncopyable {
 public:
  using ThreadInitCallback = std::function<void(EventLoop*)>;
  EventLoopThread(const ThreadInitCallback& cb = ThreadInitCallback(),
                  const std::string& name = std::string());
  ~EventLoopThread();
  EventLoop* getLoopOnlyOnce();

 private:
  void threadFunc(CountDownLatch* latch);
  EventLoop* loop_;
  CountDownLatch latch_;
  bool getLoopOnce_;
  Thread thread_;
  mutable Mutex mutex_;
  ThreadInitCallback callback_;
};

}  // namespace net
}  // namespace tmuduo

#endif  // TMUDUO_NET_EVENTLOOPTHREAD_H_