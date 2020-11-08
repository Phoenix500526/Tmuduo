#include "net/EventLoopThreadPool.h"
#include "net/EventLoop.h"
#include "net/EventLoopThread.h"

#include <stdio.h>

using namespace tmuduo;
using namespace tmuduo::net;

EventLoopThreadPool::EventLoopThreadPool(EventLoop* baseLoop,
                                         const std::string& name)
    : baseLoop_(baseLoop),
      name_(name),
      started_(false),
      numThreads_(0),
      next_(0) {}

//将析构函数定义在 .cc 文件中以避免内联
EventLoopThreadPool::~EventLoopThreadPool() = default;

// numThreads = 0 的意义何在？
// 在实际开发中，TCPServer 和 TcpClient 即有可能是一对一的关系，
// 也可能是一对多的。而 EventLoopThreadPool 作为 TcpServer 的
// 成员变量，如果不允许设置 numThreads = 0，那么 TcpServer 要
// 实现一对一的模式就比较麻烦了。
void EventLoopThreadPool::start(const ThreadInitCallback& cb) {
  assert(!started_);
  baseLoop_->assertInLoopThread();
  started_ = true;
  for (int i = 0; i < numThreads_; ++i) {
    char buf[name_.size() + 32];
    snprintf(buf, sizeof buf, "%s%d", name_.c_str(), i);
    std::unique_ptr<EventLoopThread> t(new EventLoopThread(cb, buf));
    loops_.push_back(t->getLoopOnlyOnce());
    threads_.push_back(std::move(t));
  }
  if (0 == numThreads_ && cb) {
    cb(baseLoop_);
  }
}

EventLoop* EventLoopThreadPool::getNextLoop() {
  baseLoop_->assertInLoopThread();
  assert(started_);
  EventLoop* loop = baseLoop_;
  if (!loops_.empty()) {
    loop = loops_[next_];
    ++next_;
    if (implicit_cast<size_t>(next_) >= loops_.size()) {
      next_ = 0;
    }
  }
  return loop;
}

EventLoop* EventLoopThreadPool::getLoopForHash(size_t hashCode) {
  baseLoop_->assertInLoopThread();
  EventLoop* loop = baseLoop_;
  if (!loops_.empty()) {
    loop = loops_[hashCode % loops_.size()];
  }
  return loop;
}

std::vector<EventLoop*> EventLoopThreadPool::getAllLoops() {
  baseLoop_->assertInLoopThread();
  assert(started_);
  if (loops_.empty()) {
    return std::vector<EventLoop*>(1, baseLoop_);
  } else {
    return loops_;
  }
}