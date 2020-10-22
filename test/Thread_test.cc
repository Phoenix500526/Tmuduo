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
    for (int i = 0; i < 3; ++i) {
      printf(
          "Thread name : %s,Thread ID:%d says: "
          "hellowold:%d,num:%d\n",
          CurrentThread::name(), 
          CurrentThread::tid(), i, Thread::numCreated());
      sleep(5);
    }
  }
  void memberFunc1() {
    for (int i = 0; i < 4; ++i) {
      printf(
          "Thread name : %s,  Thread ID:%d says: "
          "hellowold:%d, x = %f,num:%d\n",
          CurrentThread::name(), 
          CurrentThread::tid(), i, x_, Thread::numCreated());
      sleep(5);
    }
  }

  void memberFunc2(const std::string& text) {
    for (int i = 0; i < 5; ++i) {
      printf(
          "Thread name : %s, Thread ID:%d says: "
          "hellowold:%d, text = %s,num:%d\n",
          CurrentThread::name(),
          CurrentThread::tid(), i, text.c_str(), Thread::numCreated());
      sleep(5);
    }
  }

 private:
  double x_;
};

void func() {
  for (int i = 0; i < 6; ++i) {
    printf(
        "Thread name : %s, Thread ID:%d says: "
        "hellowold:%d,num:%d\n",
        CurrentThread::name(),
        CurrentThread::tid(), i, Thread::numCreated());
    sleep(5);
  }
}

void noname(){
  sleep(5);
  printf("Thread name : %s, Thread ID:%d says: "
        "hellowold,num:%d\n",
        CurrentThread::name(),
        CurrentThread::tid(), Thread::numCreated());
}

void monitor(){
  int num = 0;
  while(Thread::numCreated() > 1){
    if(num != Thread::numCreated()){
      printf("%d threads exist!\n",num = Thread::numCreated());
    }
    sleep(1);
  }
}

int main(void) {
  CurrentThread::t_threadName = "main";
  printf("Thread name : %s, Thread ID: %d ,num:%d\n",
         CurrentThread::name(), 
         CurrentThread::tid(), Thread::numCreated());
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
              "Thread name : %s,Thread ID:%d says: "
              "hellowold:%d,num:%d\n",
              CurrentThread::name(),
              CurrentThread::tid(), i, Thread::numCreated());
          sleep(5);
        }
      },
      "LambdaThread");
  Thread mon(&monitor, "monitor");
  sleep(4);
  Thread t6(std::move(t1));
  Thread t7(noname);
  t2.join();
  t3.join();
  t4.join();
  t5.join();
  t6.join();
  t7.join();
  mon.join();
  return 0;
}