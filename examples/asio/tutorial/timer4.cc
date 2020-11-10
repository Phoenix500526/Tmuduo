#include <stdio.h>
#include "net/EventLoop.h"
using namespace tmuduo;
using namespace tmuduo::net;

class Printer : noncopyable {
 public:
  Printer(EventLoop* loop) : loop_(loop), count_(0) {
    loop_->runEvery(1, std::bind(&Printer::print, this));
  }
  ~Printer() { printf("Final count is %d\n", count_); }
  void print() {
    if (count_ < 5) {
      printf("count = %d\n", ++count_);
    } else {
      loop_->quit();
    }
  }

 private:
  EventLoop* loop_;
  int count_;
};

int main() {
  EventLoop loop;
  Printer printer(&loop);
  loop.loop();
}