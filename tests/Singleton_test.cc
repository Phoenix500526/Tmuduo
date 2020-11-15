#include "base/Singleton.h"
#include "base/CurrentThread.h"
#include "base/Thread.h"

#include <stdio.h>

using namespace tmuduo;
using std::string;

class Test : noncopyable {
 public:
  Test() { printf("tid=%d, constructing %p\n", CurrentThread::tid(), this); }

  ~Test() {
    printf("tid=%d, destructing %p %s\n", CurrentThread::tid(), this,
           name_.c_str());
  }

  const string& name() const { return name_; }
  void setName(const string& n) { name_ = n; }

 private:
  string name_;
};

void threadFunc() {
  printf("tid=%d, %p name=%s\n", CurrentThread::tid(),
         &Singleton<Test>::instance(),
         Singleton<Test>::instance().name().c_str());
  Singleton<Test>::instance().setName("only one, changed");
}

int main() {
  Singleton<Test>::instance().setName("only one");
  Thread t1(threadFunc);
  t1.join();
  printf("tid=%d, %p name=%s\n", CurrentThread::tid(),
         &Singleton<Test>::instance(),
         Singleton<Test>::instance().name().c_str());
}
