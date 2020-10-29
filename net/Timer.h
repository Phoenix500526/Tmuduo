#ifndef TMUDUO_NET_TIMER_H_
#define TMUDUO_NET_TIMER_H_ 

#include "base/Timestamp.h"
#include "base/noncopyable.h"
#include "net/Callbacks.h"

#include <atomic>

namespace tmuduo{

namespace net{

class Timer : noncopyable
{
public:
    Timer(TimerCallback cb, Timestamp when, double interval)
        : callback_(std::move(cb)),
          expiration_(when),
          interval_(interval),
          repeat_(interval > 0.0),
          sequence_(++s_numCreated_){} 

    ~Timer() = default;

    void run() const{
        callback_();
    }

    Timestamp expiration() const{
        return expiration_;
    }

    bool repeat() const{
        return repeat_;
    }

    int64_t sequence() const{
        return sequence_;
    }

    void restart(Timestamp now);

    static int64_t numCreated() {return s_numCreated_.load();}

private:
    const TimerCallback callback_;
    Timestamp expiration_;
    const double interval_;
    const bool repeat_;
    const int64_t sequence_;

    static std::atomic<std::int64_t> s_numCreated_;
};

}   //namespace net
}   //namespace tmuduo

#endif  //TMUDUO_NET_TIMER_H_