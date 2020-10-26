#include "base/ProcessInfo.h"
#include <stdio.h>
#include "base/Thread.h"
#define __STDC_FORMAT_MACROS
#include <inttypes.h>

using namespace tmuduo;
void func() { CurrentThread::sleepUsec(1000 * 1000); }

int main() {
  printf("pid = %d\n", ProcessInfo::pid());
  printf("pid string = %s\n", ProcessInfo::pidString().c_str());
  printf("uid = %d\n", ProcessInfo::uid());
  printf("username = %s\n", ProcessInfo::username().c_str());
  printf("euid = %d\n", ProcessInfo::euid());
  printf("start time = %s\n",
         ProcessInfo::startTime().toFormattedString().c_str());
  printf("hostname = %s\n", ProcessInfo::hostname().c_str());
  printf("procname = %s\n", ProcessInfo::procname().c_str());
  printf("Current build type is %s\n",
         ProcessInfo::isDebugBuild() ? "Debug" : "Release");
  printf("opened files = %d\n", ProcessInfo::openedFiles());
  printf("before:threads = %zd\n", ProcessInfo::threads().size());
  printf("before:num threads = %d\n", ProcessInfo::numThreads());
  Thread t1(&func, "ProcessTest_thread1");
  printf("after:threads = %zd\n", ProcessInfo::threads().size());
  printf("after:num threads = %d\n", ProcessInfo::numThreads());
  printf("after:status = \n%s\n", ProcessInfo::procStatus().c_str());
  t1.join();
  return 0;
}