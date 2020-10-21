#include "base/Exception.h"
#include "base/CurrentThread.h"

namespace tmuduo {
Exception::Exception(std::string msg)
    : message_(std::move(msg)),
      stack_(CurrentThread::stackTrace(/*demangle = */ false)) {}
}