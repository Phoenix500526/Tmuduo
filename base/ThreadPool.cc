#include "base/ThreadPool.h"
#include "base/Exception.h"

#include <assert.h>

using namespace tmuduo;

ThreadPool::ThreadPool(int maxSize, const std::string& name)
    : name_(name), queue_(maxSize), running_(false) {
  assert(maxSize > 0);
}

ThreadPool::~ThreadPool() {
  if (running_) {
    stop();
  }
}

void ThreadPool::start(int numThreads) {
  assert(threads_.empty() && numThreads >= 0);
  running_ = true;
  for (int i = 0; i < numThreads; ++i) {
    threads_.emplace_back(
        new tmuduo::Thread(std::bind(&ThreadPool::runInThread, this),
                           name_ + std::to_string(i + 1)));
  }
  if (0 == numThreads && threadInitCallback_) {
    threadInitCallback_();
  }
}

void ThreadPool::stop() {
  queue_.stop();
  running_ = false;
  for (auto& thr : threads_) {
    thr->join();
  }
}

size_t ThreadPool::queueSize() const { return queue_.size(); }

void ThreadPool::run(Task&& task) { queue_.put(std::forward<Task>(task)); }

void ThreadPool::runInThread() {
  try {
    if (threadInitCallback_) {
      threadInitCallback_();
    }
    while (running_) {
      Task task;
      queue_.take(task);
      if (task) {
        task();
      }
    }
  } catch (const Exception& ex) {
    fprintf(stderr, "exception caught in ThreadPool %s\n", name_.c_str());
    fprintf(stderr, "reasion: %s\n", ex.what());
    fprintf(stderr, "stack trace: %s\n", ex.stackTrace());
    abort();
  } catch (const std::exception& ex) {
    fprintf(stderr, "exception caught in ThreadPool %s\n", name_.c_str());
    fprintf(stderr, "reasion: %s\n", ex.what());
    abort();
  } catch (...) {
    fprintf(stderr, "unknown exception caught in ThreadPool %s\n",
            name_.c_str());
    throw;
  }
}