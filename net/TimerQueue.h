#ifndef TMUDUO_NET_TIMERQUEUE_H_
#define TMUDUO_NET_TIMERQUEUE_H_

#include <set>
#include <vector>

#include "base/Mutex.h"
#include "base/Timestamp.h"
#include "net/Callbacks.h"
#include "net/Channel.h"

namespace tmuduo {
namespace net {

class EventLoop;
class Timer;
class TimerId;

// A best efforts timer queue.
// No guarantee that the callback will be on time.(尽力而为)

class TimerQueue : noncopyable {
 public:
  explicit TimerQueue(EventLoop* loop);
  ~TimerQueue();

  TimerId addTimer(TimerCallback cb, Timestamp when, double interval);

  void cancel(TimerId timerId);

 private:
  // using TimerPtr = std::unique_ptr<Timer>;
  using Entry = std::pair<Timestamp, Timer*>;
  using TimerList = std::set<Entry>;
  using ActiveTimer = std::pair<Timer*, int64_t>;
  using ActiveTimerSet = std::set<ActiveTimer>;
  void addTimerInLoop(Timer* timer);
  void cancelInLoop(TimerId timerId);

  void handleRead();

  std::vector<Entry> getExpired(Timestamp now);
  void reset(const std::vector<Entry>& expired, Timestamp now);

  bool insert(Timer* timer);

  EventLoop* loop_;
  const int timerfd_;
  Channel timerfdChannel_;
  //按照超时时间排序
  TimerList timers_;
  ActiveTimerSet activeTimers_;
  bool callingExpiredTimers_;
  ActiveTimerSet cancelingTimers_;
};

}  // namespace net
}  // namespace tmuduo

#endif  // TMUDUO_NET_TIMERQUEUE_H_