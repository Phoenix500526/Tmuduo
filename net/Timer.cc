#include "net/Timer.h"

using namespace tmuduo;
using namespace tmuduo::net;

std::atomic<std::int64_t> Timer::s_numCreated_;

void Timer::restart(Timestamp now){
    if(repeat_){
        expiration_ = addTime(now, interval_);
    }else{
        expiration_ = Timestamp::invalid();
    }
}