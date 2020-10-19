#ifndef TMUDUO_BASE_COUNTDOWNLATCH_H_
#define TMUDUO_BASE_COUNTDOWNLATCH_H_ 

#include "base/Mutex.h"
#include "base/Condition.h"

namespace tmuduo{
class CountDownLatch:noncopyable
{
public:
    explicit CountDownLatch(int count);
    void wait();
    void countDown();
    int getCount() const;
private:
    mutable Mutex mutex_;
    Condition condition_ GUARDED_BY(mutex_);    
    int count_ GUARDED_BY(mutex_);
};
}   //namespace tmuduo{
#endif  //TMUDUO_BASE_COUNTDOWNLATCH_H_