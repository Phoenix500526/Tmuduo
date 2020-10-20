#include "base/CountDownLatch.h"

using namespace tmuduo;

CountDownLatch::CountDownLatch(int count)
    : mutex_(), condition_(), count_(count) {}

void CountDownLatch::wait() {
  UniqueLock lock(mutex_);
  while (count_ > 0) condition_.wait(lock);
}

void CountDownLatch::countDown() {
  UniqueLock lock(mutex_);
  --count_;
  if (0 == count_) condition_.notify_all();
}

int CountDownLatch::getCount() const {
  UniqueLock lock(mutex_);
  return count_;
}