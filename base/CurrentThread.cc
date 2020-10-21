#include "base/CurrentThread.h"

#include <cxxabi.h>
#include <execinfo.h>
#include <stdlib.h>

namespace tmuduo {
namespace CurrentThread {
__thread std::size_t t_cachedTid = 0;
__thread char t_tidString[32];
__thread int t_tidStringLength = 6;
__thread const char* t_threadName = "unknown";

std::string stackTrace(bool demangle) {
  std::string stack;
  const int max_frames = 200;
  void* frame[max_frames];
  // int backtrace (void **buffer, int
  // size):用来获取当前线程的调用堆栈，获取的信息将会被存放在buffer中，它是一个指针数组。
  //参数size用来指定buffer中可以保存多少个void*
  //元素。函数返回值是实际获取的指针个数，最大不超过size大小
  //在buffer中的指针实际是从堆栈中获取的返回地址,每一个堆栈框架有一个返回地址。
  //注意某些编译器的优化选项对获取正确的调用堆栈有干扰，另外内联函数没有堆栈框架；删除框架指针也会使无法正确解析堆栈内容。
  int nptrs = ::backtrace(frame, max_frames);
  char** strings = ::backtrace_symbols(frame, nptrs);
  if (strings) {
    size_t len = 256;
    char* demangled = demangle ? static_cast<char*>(::malloc(len)) : nullptr;
    for (int i = 1; i < nptrs; ++i) {
      if (demangle) {
        char* left_par = nullptr;
        char* plus = nullptr;
        for (char* p = strings[i]; *p; ++p) {
          if (*p == '(')
            left_par = p;
          else if (*p == '+')
            plus = p;
        }
        if (left_par && plus) {
          *plus = '\0';
          int status = 0;
          char* ret =
              abi::__cxa_demangle(left_par + 1, demangled, &len, &status);
          *plus = '+';
          if (status == 0) {
            demangled = ret;
            stack.append(strings[i], left_par + 1);
            stack.append(demangled);
            stack.append(plus);
            stack.push_back('\n');
            continue;
          }
        }
      }
      stack.append(strings[i]);
      stack.push_back('\n');
    }
    free(demangled);
    free(strings);
  }
  return stack;
}
}  // namespace CurrentThread
}  // namespace tmuduo