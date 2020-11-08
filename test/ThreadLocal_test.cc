#include "base/Thread.h"
#include "base/CurrentThread.h"

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

thread_local Test testObj1;
thread_local Test testObj2;

void print() {
  printf("tid=%d, obj1 %p name=%s\n", CurrentThread::tid(), &testObj1,
         testObj1.name().c_str());
  printf("tid=%d, obj2 %p name=%s\n", CurrentThread::tid(), &testObj2,
         testObj2.name().c_str());
}

void threadFunc() {
  print();
  testObj1.setName("changed 1");
  testObj2.setName("changed 42");
  print();
}

int main() {
  testObj1.setName("main one");
  print();
  Thread t1(threadFunc);
  t1.join();
  testObj2.setName("main two");
  print();
}
