#include <stdio.h>
#include "base/Mutex.h"
#include "net/EventLoop.h"
#include "net/EventLoopThread.h"

using namespace tmuduo;
using namespace tmuduo::net;

class Printer : noncopyable {
 public:
  Printer(EventLoop* loop1, EventLoop* loop2)
      : loop1_(loop1), loop2_(loop2), count_(0) {
    loop1_->runEvery(1, std::bind(&Printer::print1, this));
    loop2_->runEvery(1, std::bind(&Printer::print2, this));
  }
  ~Printer() { printf("Final count is %d\n", count_); }
  void print1() {
    UniqueLock lock(mutex_);
    if (count_ < 10) {
      printf("Timer1: count = %d\n", ++count_);
    } else {
      loop1_->quit();
    }
  }
  void print2() {
    UniqueLock lock(mutex_);
    if (count_ < 10) {
      printf("Timer2: count = %d\n", ++count_);
    } else {
      loop2_->quit();
    }
  }

 private:
  Mutex mutex_;
  EventLoop* loop1_ GUARDED_BY(mutex_);
  EventLoop* loop2_ GUARDED_BY(mutex_);
  int count_ GUARDED_BY(mutex_);
};

int main() {
  EventLoop loop;
  EventLoopThread loopThread;
  EventLoop* loopInAnotherThread = loopThread.getLoopOnlyOnce();
  std::unique_ptr<Printer> printer(new Printer(&loop, loopInAnotherThread));
  loop.loop();
}