#include "base/Exception.h"
#include <stdio.h>
#include <functional>
#include <vector>
#include "base/CurrentThread.h"

class Bar {
 public:
  void test(bool flag, std::vector<std::string> names = {}) {
    printf("Stack:\n%s\n", tmuduo::CurrentThread::stackTrace(flag).c_str());
    [flag] {
      printf("Stack inside lambda:\n%s\n",
             tmuduo::CurrentThread::stackTrace(flag).c_str());
    }();
    std::function<void()> func([flag] {
      printf("Stack inside std::function:\n%s\n",
             tmuduo::CurrentThread::stackTrace(flag).c_str());
    });
    func();
    func = std::bind(&Bar::callback, this, flag);
    func();
    throw tmuduo::Exception("oops");
  }

 private:
  void callback(bool flag) {
    printf("Stack inside std::bind:\n%s\n",
           tmuduo::CurrentThread::stackTrace(flag).c_str());
  }
};

void foo(bool flag) {
  Bar b;
  b.test(flag);
}

int main(int agrc, char* agrv[]) {
  try {
    if (agrc > 1) {
      foo(false);
    }
    foo(true);
  } catch (const tmuduo::Exception& ex) {
    printf("reason:%s\n", ex.what());
    printf("stack trace:\n%s\n", ex.stackTrace());
  }
}