#ifndef TMUDUO_BASE_LOGSTREAM_H_
#define TMUDUO_BASE_LOGSTREAM_H_

#include "base/StringPiece.h"
#include "base/TypeCast.h"
#include "base/noncopyable.h"

#include <assert.h>
#include <string>

namespace tmuduo {

namespace detail {

constexpr int kSmallBuffer = 4000;
constexpr int kLargeBuffer = 4000 * 1000;

template <int SIZE>
class FixedBuffer : noncopyable {
 public:
  // cookies 是一个函数指针，它指向了两个没有任何功能的函数 cookieStart 和
  // cookiesEnd，这两个函数
  //相当于页眉和页脚，当程序崩溃时，可以利用 gdb 把 cookieStart
  //的值打印出来，并在 core dump 文件
  //中搜查对应的地址。这样就可以找到崩溃后遗留在内存中尚未刷到文件中的日志信息
  FixedBuffer() : cur_(data_) { setCookie(cookieStart); }
  ~FixedBuffer() { setCookie(cookieEnd); }
  void append(const char* buf, size_t len) {
    if (implicit_cast<size_t>(avail()) > len) {
      memcpy(cur_, buf, len);
      cur_ += len;
    }
  }
  const char* data() const { return data_; }
  int length() const { return static_cast<int>(cur_ - data_); }
  char* current() { return cur_; }
  int avail() const { return static_cast<int>(end() - cur_); }
  void add(size_t len) { cur_ += len; }
  void reset() { cur_ = data_; }
  void bzero() { memZero(data_, sizeof data_); }

  void setCookie(void (*cookie)()) { cookie_ = cookie; }

  // for used by GDB
  const char* debugString();

  // for used by unit test
  std::string toString() const { return std::string(data_, length()); }
  StringPiece toStringPiece() const { return StringPiece(data_, length()); }

 private:
  const char* end() const { return data_ + sizeof data_; }
  // cookieStart 与 cookieEnd 必须是非内联函数
  static void cookieStart();
  static void cookieEnd();
  void (*cookie_)();
  char data_[SIZE];
  char* cur_;
};

}  // namespace detail

class LogStream : noncopyable {
  using self = LogStream;

 public:
  using Buffer = detail::FixedBuffer<detail::kSmallBuffer>;
  LogStream() = default;
  ~LogStream() = default;
  self& operator<<(bool v) {
    buffer_.append(v ? "1" : "0", 1);
    return *this;
  }
  self& operator<<(short);
  self& operator<<(unsigned short);
  self& operator<<(int);
  self& operator<<(unsigned int);
  self& operator<<(long);
  self& operator<<(unsigned long);
  self& operator<<(long long);
  self& operator<<(unsigned long long);
  self& operator<<(const void*);
  self& operator<<(float v) {
    *this << static_cast<double>(v);
    return *this;
  }
  self& operator<<(double);
  self& operator<<(char v) {
    buffer_.append(&v, 1);
    return *this;
  }
  self& operator<<(const char* str) {
    if (str) {
      buffer_.append(str, strlen(str));
    } else {
      buffer_.append("(null)", 6);
    }
    return *this;
  }
  self& operator<<(const unsigned char* str) {
    *this << reinterpret_cast<const char*>(str);
    return *this;
  }
  self& operator<<(const std::string& v) {
    buffer_.append(v.c_str(), v.size());
    return *this;
  }
  self& operator<<(const StringPiece& v) {
    buffer_.append(v.data(), v.size());
    return *this;
  }
  self& operator<<(const Buffer& v) {
    *this << v.toStringPiece();
    return *this;
  }
  void append(const char* data, int len) { buffer_.append(data, len); }
  const Buffer& buffer() const { return buffer_; }
  void resetBuffer() { buffer_.reset(); }

 private:
  void staticCheck();
  template <typename T>
  void formatInteger(T);
  Buffer buffer_;
  static const int kMaxNumericSize = 32;
};

class Fmt {
 public:
  template <typename T>
  Fmt(const char* fmt, T val);
  const char* data() const { return buf_; }
  int length() const { return length_; }

 private:
  char buf_[32];
  int length_;
};

inline LogStream& operator<<(LogStream& s, const Fmt& fmt) {
  s.append(fmt.data(), fmt.length());
  return s;
}

}  // namespace tmuduo

#endif  // TMUDUO_BASE_LOGSTREAM_H_