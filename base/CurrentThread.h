#ifndef TMUDUO_BASE_CURRENTTHREAD_H_
#define TMUDUO_BASE_CURRENTTHREAD_H_

#include <stddef.h>
#include <stdint.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <string>

namespace tmuduo {
namespace CurrentThread {

extern thread_local pid_t t_cachedTid;
extern thread_local char t_tidString[32];
extern thread_local int t_tidStringLength;
extern thread_local const char* t_threadName;

// cacheTid、isMainTHread、sleepUsec均定义在 Thread.cc 文件中，
// 因为需要拿到对应线程的 ID
void cacheTid();
bool isMainThread();
void sleepUsec(int64_t usec);  // for testing
void clearCache();

//使用 detail 来将 gettid 和 CurrentThread 中其他成员隔离开来
namespace detail {

inline pid_t gettid() { return static_cast<pid_t>(::syscall(SYS_gettid)); }

}  // namespace detail

inline pid_t tid() {
  if (__builtin_expect(t_cachedTid == 0, 0)) {
    cacheTid();
  }
  return t_cachedTid;
}

inline const char* tidString() { return t_tidString; }

// inline int tidStringLength() { return t_tidStringLength; }

inline const char* name() { return t_threadName; }

std::string stackTrace(bool demangle);
}  // namespace CurrentThread
}  // namespace tmuduo
#endif  // TMUDUO_BASE_CURRENTTHREAD_H_