#ifndef TMUDUO_BASE_CURRENTTHREAD_H_
#define TMUDUO_BASE_CURRENTTHREAD_H_

#include <stddef.h>
#include <stdint.h>
#include <string>

namespace tmuduo {
namespace CurrentThread {

extern __thread std::size_t t_cachedTid;  //利用 hash 来转换
extern __thread char t_tidString[32];
extern __thread int t_tidStringLength;
extern __thread const char* t_threadName;

// cacheTid、isMainTHread、sleepUsec均定义在 Thread.cc 文件中，
// 因为需要拿到对应线程的 ID
void cacheTid();
bool isMainThread();
void sleepUsec(int64_t usec);

inline std::size_t tid() {
  if (__builtin_expect(t_cachedTid == 0, 0)) {
    cacheTid();
  }
  return t_cachedTid;
}

inline const char* tidString() { return t_tidString; }

inline int tidStringLength() { return t_tidStringLength; }

inline const char* name() { return t_threadName; }

std::string stackTrace(bool demangle);
}  // namespace CurrentThread
}  // namespace tmuduo
#endif  // TMUDUO_BASE_CURRENTTHREAD_H_