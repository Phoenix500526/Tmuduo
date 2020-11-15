#include "net/EventLoopThread.h"
#include "base/Thread.h"
#include "net/EventLoop.h"

using namespace tmuduo;
using namespace tmuduo::net;

void print(EventLoop* p = nullptr) {
  printf("print:pid = %d, tid = %d, loop = %p\n", getpid(),
         CurrentThread::tid(), p);
}

void init1(EventLoop* p = nullptr) { CurrentThread::sleepUsec(10000 * 1000); }

void init2(EventLoop* p = nullptr) { CurrentThread::sleepUsec(8000 * 1000); }

void quit(EventLoop* p) {
  print(p);
  p->quit();
}

int main() {
  print();
  { EventLoopThread thr1(init1, "EvLoopThread1"); }
  {
    EventLoopThread thr2(init2, "EvLoopThread2");
    EventLoop* loop = thr2.getLoopOnlyOnce();
    loop->runInLoop(std::bind(print, loop));
    CurrentThread::sleepUsec(500 * 1000);
  }
  {
    EventLoopThread thr3;
    EventLoop* loop = thr3.getLoopOnlyOnce();
    loop->runInLoop(std::bind(quit, loop));
    CurrentThread::sleepUsec(500 * 1000);
  }
}