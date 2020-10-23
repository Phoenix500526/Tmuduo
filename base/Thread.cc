#include "base/Thread.h"
#include <sys/prctl.h>
#include "base/Exception.h"

namespace tmuduo {

std::atomic<int> Thread::numCreated_(0);

void Thread::runInThread(ThreadFunc func, const std::string& name) {
  CurrentThread::t_cachedTid = CurrentThread::tid();
  if (name.empty()) {
    char buf[32];
    snprintf(buf, sizeof buf, "Thread%d", Thread::numCreated());
    CurrentThread::t_threadName = buf;
  } else {
    CurrentThread::t_threadName = name.c_str();
  }
  //::prctl(PR_SET_NAME, threadName)：表示用 threadName
  //为当前线程命名，threadName 的长度
  //不得超过 16 bytes。当名字长度超过 16 个字节时会默认截断
  ::prctl(PR_SET_NAME, CurrentThread::t_threadName);
  try {
    func();
    CurrentThread::t_threadName = "finished";
  } catch (const Exception& ex) {
    CurrentThread::t_threadName = "crashed";
    fprintf(stderr, "exception caught in Thread %s\n", name.c_str());
    fprintf(stderr, "reason: %s\n", ex.what());
    fprintf(stderr, "stack trace: %s\n", ex.stackTrace());
    abort();
  } catch (const std::exception& ex) {
    CurrentThread::t_threadName = "crashed";
    fprintf(stderr, "exception caught in Thread %s\n", name.c_str());
    fprintf(stderr, "reason: %s\n", ex.what());
    abort();
  } catch (...) {
    CurrentThread::t_threadName = "crashed";
    fprintf(stderr, "unknown exception caught in Thread %s\n", name.c_str());
    throw;  // rethrow
  }
}

Thread::Thread(ThreadFunc func, const std::string& name)
    : thread_(&Thread::runInThread, this, std::move(func), name) {
  ++numCreated_;
}

Thread::Thread(Thread&& rhs) noexcept : thread_(std::move(rhs.thread_)) {}

Thread& Thread::operator=(Thread&& rhs) noexcept {
  if (this != &rhs) {
    thread_ = std::move(rhs.thread_);
  }
  return *this;
}

void Thread::join() {
  if (thread_.joinable()) {
    --numCreated_;
    thread_.join();
  }
}

Thread::~Thread() {
  if (thread_.joinable()) {
    --numCreated_;
    thread_.detach();
  }
}

}  // namespace tmuduo