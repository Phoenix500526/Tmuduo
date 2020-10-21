#include "base/Exception.h"
#include <stdio.h>
#include <functional>
#include <vector>
#include "base/CurrentThread.h"

class Bar {
 public:
  void test(std::vector<std::string> names = {}) {
    printf("Stack:\n%s\n", tmuduo::CurrentThread::stackTrace(true).c_str());
    [] {
      printf("Stack inside lambda:\n%s\n",
             tmuduo::CurrentThread::stackTrace(true).c_str());
    }();
    std::function<void()> func([] {
      printf("Stack inside std::function:\n%s\n",
             tmuduo::CurrentThread::stackTrace(true).c_str());
    });
    func();
    func = std::bind(&Bar::callback, this);
    func();
    throw tmuduo::Exception("oops");
  }

 private:
  void callback() {
    printf("Stack inside std::bind:\n%s\n",
           tmuduo::CurrentThread::stackTrace(true).c_str());
  }
};

void foo() {
  Bar b;
  b.test();
}

int main() {
  try {
    foo();
  } catch (const tmuduo::Exception& ex) {
    printf("reason:%s\n", ex.what());
    printf("stack trace:\n%s\n", ex.stackTrace());
  }
}