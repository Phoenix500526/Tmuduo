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
    bool shouldQuit = false;
    int count = 0;
    {
      UniqueLock lock(mutex_);
      if (count_ < 10) {
        count = count_;
        ++count_;
      } else {
        shouldQuit = true;
      }
    }
    if (shouldQuit) {
      loop1_->quit();
    } else {
      printf("Timer 1 : %d\n", count);
    }
  }

  void print2() {
    bool shouldQuit = false;
    int count = 0;
    {
      UniqueLock lock(mutex_);
      if (count_ < 10) {
        count = count_;
        ++count_;
      } else {
        shouldQuit = true;
      }
    }
    if (shouldQuit) {
      loop2_->quit();
    } else {
      printf("Timer 2 : %d\n", count);
    }
  }

 private:
  Mutex mutex_;
  EventLoop* loop1_;
  EventLoop* loop2_;
  int count_ GUARDED_BY(mutex_);
};

int main() {
  EventLoop loop;
  EventLoopThread loopThread;
  EventLoop* loopInAnotherThread = loopThread.getLoopOnlyOnce();
  std::unique_ptr<Printer> printer(new Printer(&loop, loopInAnotherThread));
  loop.loop();
}