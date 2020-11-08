#ifndef TMUDUO_BASE_THREADLOCALSINGLETON_H_
#define TMUDUO_BASE_THREADLOCALSINGLETON_H_

#include <assert.h>
#include "base/noncopyable.h"

namespace tmuduo {

template <typename T>
class ThreadLocalSingleton : noncopyable {
 public:
  ThreadLocalSingleton() = delete;
  ~ThreadLocalSingleton() = delete;
  static T& instance() {
    thread_local T instance;
    return instance;
  }
};

}  // namespace tmuduo

#endif  // TMUDUO_BASE_THREADLOCALSINGLETON_H_