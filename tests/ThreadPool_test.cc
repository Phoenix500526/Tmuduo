#include "base/ThreadPool.h"
#include "base/CountDownLatch.h"
#include "base/CurrentThread.h"
#include "base/Logging.h"

#include <stdio.h>
using namespace tmuduo;

void print() { printf("tid = %d\n", CurrentThread::tid()); }

void printString(const std::string& str) {
  LOG_INFO << str;
  usleep(100 * 100);
}

void longTask(int num) {
  LOG_INFO << "longTask" << num;
  CurrentThread::sleepUsec(3000000);
}

void test(int maxSize) {
  LOG_WARN << "Test ThreadPool with max queue size = " << maxSize;
  ThreadPool pool(maxSize, "MainThreadPool");
  pool.start(5);
  LOG_WARN << "Adding";
  pool.run(print);
  pool.run(print);
  for (int i = 0; i < 100; ++i) {
    char buf[32];
    snprintf(buf, sizeof buf, "task %d", i + 1);
    pool.run(std::bind(printString, std::string(buf)));
  }
  LOG_WARN << "Done";
  CountDownLatch latch(1);
  pool.run(std::bind(&CountDownLatch::countDown, &latch));
  latch.wait();
  pool.stop();
}

void test2() {
  LOG_WARN << "Test ThreadPool by stoping early. tid = "
           << CurrentThread::tid();
  ThreadPool pool(5);
  pool.start(3);
  Thread t1(
      [&pool]() {
        LOG_WARN << "The tid of thread t1 is " << CurrentThread::tid();
        for (int i = 0; i < 20; ++i) {
          pool.run(std::bind(longTask, i));
        }
      },
      "thread1");
  CurrentThread::sleepUsec(5000000);
  LOG_WARN << "stop pool";
  pool.stop();
  t1.join();
  pool.run(print);
  LOG_WARN << "test2 Done";
}

int main(void) {
  test(1);
  test(5);
  test(10);
  test(50);
  test2();
  return 0;
}