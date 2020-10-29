#ifndef TMUDUO_NET_EVENTLOOP_H_
#define TMUDUO_NET_EVENTLOOP_H_

#include <atomic>
#include <functional>
#include <vector>

#include "base/CurrentThread.h"
#include "base/Mutex.h"
#include "base/Timestamp.h"
#include "net/Callbacks.h"
#include "net/TimerId.h"

#include <boost/any.hpp>

namespace tmuduo {

namespace net {

class Channel;
class Poller;
class TimerQueue;

class EventLoop {
 public:
  using Functor = std::function<void()>;

  EventLoop();
  ~EventLoop();
  void loop();
  //退出循环,如果使用 shared_ptr<EventLoop>
  //来调用则是线程安全的,如果通过裸指针来调用则不是线程安全的
  void quit();

  Timestamp pollReturnTime() const { return pollReturnTime_; }

  int64_t iteration() const { return iteration_; }

  void runInLoop(Functor cb);

  void queueInLoop(Functor cb);
  size_t queueSize() const;

  // timers
  TimerId runAt(Timestamp time, TimerCallback cb);
  TimerId runAfter(double delay, TimerCallback cb);
  TimerId runEvery(double interval, TimerCallback cb);

  // cancel the timer
  void cancel(TimerId timerId);

  // internal usage
  void wakeup();
  void updateChannel(Channel* channel);
  void removeChannel(Channel* channel);
  bool hasChannel(Channel* channel);

  void assertInLoopThread() {
    if (!isInLoopThread()) {
      abortNotInLoopThread();
    }
  }

  bool isInLoopThread() const { return threadId_ == CurrentThread::tid(); }
  bool eventHandling() const { return eventHandling_; }

  void setContext(const boost::any& context) { context_ = context; }

  const boost::any& getContext() const { return context_; }

  boost::any* getMutableContext() { return &context_; }

  static EventLoop* getEventLoopOfCurrentThread();

 private:
  using ChannelList = std::vector<Channel*>;

  void abortNotInLoopThread();
  void handleRead();
  void doPendingFunctors();
  void printActiveChannels() const;  //测试时使用

  bool looping_;
  std::atomic<bool> quit_;
  bool eventHandling_;
  bool callingPendingFunctors_;
  int64_t iteration_;
  const pid_t threadId_;
  Timestamp pollReturnTime_;
  std::unique_ptr<Poller> poller_;
  std::unique_ptr<TimerQueue> timerQueue_;

  int wakeupFd_;
  std::unique_ptr<Channel> wakeupChannel_;
  boost::any context_;

  // scratch variables
  ChannelList activeChannels_;
  Channel* currentActiveChannel_;

  mutable Mutex mutex_;
  std::vector<Functor> pendingFunctors_ GUARDED_BY(mutex_);
};

}  // namespace net
}  // namespace tmuduo

#endif  // TMUDUO_NET_EVENTLOOP_H_