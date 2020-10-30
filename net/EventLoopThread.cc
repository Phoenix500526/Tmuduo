#include "net/EventLoopThread.h"
#include "net/EventLoop.h"

using namespace tmuduo;
using namespace tmuduo::net;

EventLoopThread::EventLoopThread(const ThreadInitCallback& cb,
                                 const std::string& name)
    : loop_(nullptr),
      latch_(1),
      getLoopOnce_(true),
      thread_(std::bind(&EventLoopThread::threadFunc, this, &latch_), name),
      mutex_(),
      callback_(cb) {}

EventLoopThread::~EventLoopThread() {
  if (loop_ != nullptr) {
    loop_->quit();
    thread_.join();
  }
}

// EventLoop 的模式是先启动循环,然后将回调函数注入.由于线程可能已经进入了 loop
// 循环,因此需要让主线程来向 EventLoop 中注入回调.而注入回调就必须有 EventLoop
// 指针
EventLoop* EventLoopThread::getLoopOnlyOnce() {
  assert(getLoopOnce_);
  getLoopOnce_ = false;
  latch_.wait();
  assert(loop_ != nullptr);
  return loop_;
}

//
void EventLoopThread::threadFunc(CountDownLatch* latch) {
  EventLoop loop;
  if (callback_) {
    callback_(&loop);
  }
  loop_ = &loop;
  latch->countDown();
  loop.loop();
  // loop 中有一些函数是可以给其他线程使用的,而有一些是可以给其他线程使用的
  //因此需要确保在 threadFunc 能够独占这个 loop_ 的情况下,才可以改变 loop_
  UniqueLock lock(mutex_);
  loop_ = nullptr;
}