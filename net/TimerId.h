#ifndef TMUDUO_NET_TIMERID_H_
#define TMUDUO_NET_TIMERID_H_

#include "base/copyable.h"
namespace tmuduo {

namespace net {

class Timer

    class TimerId : copyable {
 public:
  TimerId() : timer_(nullptr), sequence_(0) {}
  TimerId(Timer* timer, int64_t seq) : timer_(timer), sequence_(seq) {}
  ~TimerId() = default;
  friend class TimerQueue;

 private:
  Timer* timer_;
  int64_t sequence_;
};

}  // namespace net

}  // namespace tmuduo

#endif  // TMUDUO_NET_TIMERID_H_