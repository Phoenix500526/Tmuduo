#include "base/Thread.h"
#include <unistd.h>
#include <iostream>

using namespace std;
using namespace tmuduo;

class Foo {
 public:
  explicit Foo(double x) : x_(x) {}
  ~Foo() = default;
  void operator()() {
    for (int i = 0; i < 10; ++i) {
      printf(
          "Thread name : %s, Thread String: %s, Thread ID:%d says: "
          "hellowold:%d\n",
          CurrentThread::t_threadName, CurrentThread::t_tidString,
          CurrentThread::t_cachedTid, i);
      sleep(5);
    }
  }
  void memberFunc1() {
    for (int i = 0; i < 15; ++i) {
      printf(
          "Thread name : %s, Thread String: %s, Thread ID:%d says: "
          "hellowold:%d, x = %f\n",
          CurrentThread::t_threadName, CurrentThread::t_tidString,
          CurrentThread::t_cachedTid, i, x_);
      sleep(5);
    }
  }

  void memberFunc2(const std::string& text) {
    for (int i = 0; i < 3; ++i) {
      printf(
          "Thread name : %s, Thread String: %s, Thread ID:%d says: "
          "hellowold:%d, text = %s\n",
          CurrentThread::t_threadName, CurrentThread::t_tidString,
          CurrentThread::t_cachedTid, i, text.c_str());
      sleep(5);
    }
  }

 private:
  double x_;
};

void func() {
  for (int i = 0; i < 8; ++i) {
    printf(
        "Thread name : %s, Thread String: %s, Thread ID:%d says: "
        "hellowold:%d\n",
        CurrentThread::t_threadName, CurrentThread::t_tidString,
        CurrentThread::t_cachedTid, i);
    sleep(5);
  }
}

int main(void) {
  CurrentThread::t_threadName = "main";
  printf("Thread name : %s,Thread String: %s,Thread ID:%d\n",
         CurrentThread::t_threadName, CurrentThread::t_tidString,
         CurrentThread::tid());
  Foo test(5.0);
  Thread::ThreadFunc FooOperator = std::bind(&Foo::operator(), &test);
  Thread t1(FooOperator, "FooOperator");
  Thread::ThreadFunc FooMem1 = std::bind(&Foo::memberFunc1, &test);
  Thread t2(FooMem1, "FooMem1");
  Thread::ThreadFunc FooMem2 =
      std::bind(&Foo::memberFunc2, &test, "I'm Foo::memberFunc2");
  Thread t3(FooMem2, "FooMem2");

  Thread t4(func, "freefunction");
  Thread t5(
      []() {
        for (int i = 0; i < 5; ++i) {
          printf(
              "Thread name : %s,Thread String: %s,Thread ID:%d says: "
              "hellowold:%d\n",
              CurrentThread::t_threadName, CurrentThread::t_tidString,
              CurrentThread::t_cachedTid, i);
          sleep(5);
        }
      },
      "LambdaThread");
  sleep(4);
  Thread t6(std::move(t1));
  t2.join();
  t3.join();
  t4.join();
  t5.join();
  t6.join();
  return 0;
}