#ifndef TMUDUO_BASE_EXCEPTION_H_
#define TMUDUO_BASE_EXCEPTION_H_

#include <exception>
#include <string>
namespace tmuduo {

class Exception : public std::exception {
 public:
  Exception(std::string what);
  ~Exception() noexcept override = default;

  const char* what() const noexcept override { return message_.c_str(); }

  const char* stackTrace() const noexcept { return stack_.c_str(); }

 private:
  std::string message_;
  std::string stack_;
};

}  // namespace tmuduo

#endif  // TMUDUO_BASE_EXCEPTION_H_