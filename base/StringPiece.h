// Taken from PCRE pcre_stringpiece.h

// Copyright (c) 2005, Google Inc.
// All rights reserved.

// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:

//     * Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above
// copyright notice, this list of conditions and the following disclaimer
// in the documentation and/or other materials provided with the
// distribution.
//     * Neither the name of Google Inc. nor the names of its
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.

// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

// Author: Sanjay Ghemawat

#ifndef TMUDUO_BASE_STRINGPIECE_H_
#define TMUDUO_BASE_STRINGPIECE_H_

#include <string.h>

#ifndef NDEBUG
#include <assert.h>
#endif

namespace tmuduo {
// You can use StringPiece as a function or method parameter. A StringPiece
// parameter can receive a double-quoted std::string literal argument, a “const
// char*” argument, a std::string argument, or a StringPiece argument with no
// data
// copying. Systematic use of StringPiece for arguments reduces data
// copies and strlen() calls.
// StringPiece 存在的意义有：
// 1. 统一参数格式
// 2. 减少不必要的拷贝，以及简化 strlen 的调用
// StringPiece 适用于以下两种情况：
// 1.
// 函数参数传入了string，而该函数内调用的另一个函数需要接收该string的一个substring
// 2. 函数参数传入了string，而该函数需要return一个该string的substring
class StringPiece {
 public:
  StringPiece() : ptr_(nullptr), length_(0) {}
  StringPiece(const char* str) : ptr_(str), length_(strlen(str)) {}
  StringPiece(const unsigned char* str)
      : ptr_(reinterpret_cast<const char*>(str)), length_(strlen(ptr_)) {}
  StringPiece(const std::string& str)
      : ptr_(str.c_str()), length_(str.size()) {}
  StringPiece(const char* offset, int len) : ptr_(offset), length_(len) {}
  const char* data() const { return ptr_; }
  size_t size() const { return length_; }
  bool empty() const { return length_ == 0; }
  const char* begin() const { return ptr_; }
  const char* end() const { return ptr_ + length_; }

  void clear() {
    ptr_ = nullptr;
    length_ = 0;
  }
  void set(const char* buf, size_t len) {
    ptr_ = buf;
    length_ = len;
  }
  void set(const char* buf) {
    ptr_ = buf;
    length_ = strlen(ptr_);
  }
  void set(const void* buf) {
    ptr_ = static_cast<const char*>(buf);
    length_ = strlen(ptr_);
  }
  void remove_prefix(size_t n) {
    assert(n < length_);
    ptr_ += n;
    length_ -= n;
  }
  void remove_suffix(size_t n) {
    assert(n < length_);
    length_ -= n;
  }
  char operator[](size_t i) const { return ptr_[i]; }

  bool operator==(const StringPiece& str) const {
    return ((str.length_ == length_) && (memcmp(ptr_, str.ptr_, length_) == 0));
  }

  bool operator!=(const StringPiece& str) const { return !(*this == str); }

  std::string as_string() const { return std::string(ptr_, length_); }

  bool start_with(const StringPiece& str) const {
    return (length_ >= str.length_) &&
           (0 == memcmp(ptr_, str.ptr_, str.length_));
  }

//定义 StringPiece 的二元谓词
#define STRINGPIECE_BINARY_PREDICATE(cmp, auxcmp)                              \
  bool operator cmp(const StringPiece& str) const {                            \
    int r =                                                                    \
        memcmp(ptr_, str.ptr_, length_ < str.length_ ? length_ : str.length_); \
    return ((r auxcmp 0)) || ((0 == r) && (length_ cmp str.length_));          \
  }
  STRINGPIECE_BINARY_PREDICATE(<, <)
  STRINGPIECE_BINARY_PREDICATE(<=, <)
  STRINGPIECE_BINARY_PREDICATE(>, >)
  STRINGPIECE_BINARY_PREDICATE(>=, >)
#undef STRINGPIECE_BINARY_PREDICATE
  int compare(const StringPiece& str) {
    int r =
        memcmp(ptr_, str.ptr_, length_ < str.length_ ? length_ : str.length_);
    if (0 == r) {
      if (length_ < str.length_)
        r = -1;
      else if (length_ > str.length_)
        r = 1;
    }
    return r;
  }

 private:
  const char* ptr_;
  size_t length_;  //使用
};
}  // namespace tmuduo

#endif  // TMUDUO_BASE_STRINGPIECE_H_