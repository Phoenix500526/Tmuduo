#ifndef TMUDUO_NET_BUFFER_H_
#define TMUDUO_NET_BUFFER_H_

#include "base/StringPiece.h"
#include "base/TypeCast.h"
#include "base/copyable.h"
#include "net/Endian.h"

#include <assert.h>
#include <string.h>
#include <algorithm>
#include <vector>

namespace tmuduo {

namespace net {

/// A buffer class modeled after org.jboss.netty.buffer.ChannelBuffer
///
/// @code
/// +-------------------+------------------+------------------+
/// | prependable bytes |  readable bytes  |  writable bytes  |
/// |                   |     (CONTENT)    |                  |
/// +-------------------+------------------+------------------+
/// |                   |                  |                  |
/// 0      <=      readerIndex   <=   writerIndex    <=     size
/// @endcode

// 在 muduo 网络库中，Buffer 的前方添加了prepend 空间，这样程序就能很方便地
// 在 Buffer 中数据的前面添加几个字节
class Buffer : public tmuduo::copyable {
 public:
  static const size_t kCheapPrepend = 8;
  static const size_t kInitialSize = 1024;

  explicit Buffer(size_t initailSize = kInitialSize)
      : buffer_(kCheapPrepend + initailSize),
        readerIndex_(kCheapPrepend),
        writerIndex_(kCheapPrepend) {
    assert(0 == readableBytes());
    assert(initailSize == writableBytes());
    assert(prependableBytes() == kCheapPrepend);
  }

  size_t readableBytes() const { return writerIndex_ - readerIndex_; }

  size_t writableBytes() const { return buffer_.size() - writerIndex_; }

  size_t prependableBytes() const { return readerIndex_; }

  void swap(Buffer& rhs) {
    buffer_.swap(rhs.buffer_);
    std::swap(readerIndex_, rhs.readerIndex_);
    std::swap(writerIndex_, rhs.writerIndex_);
  }
  //用于窥探 readableBytes 区域中的数据
  const char* peek() const { return begin() + readerIndex_; }

  const char* findCRLF() const {
    const char* crlf = std::search(peek(), beginWrite(), kCRLF, kCRLF + 2);
    return crlf == beginWrite() ? nullptr : crlf;
  }

  const char* findCRLF(const char* start) const {
    assert(peek() <= start);
    assert(start <= beginWrite());
    const char* crlf = std::search(start, beginWrite(), kCRLF, kCRLF + 2);
    return crlf == beginWrite() ? nullptr : crlf;
  }

  const char* findEOL() const {
    const void* eol = memchr(peek(), '\n', readableBytes());
    return static_cast<const char*>(eol);
  }

  const char* findEOL(const char* start) const {
    assert(peek() <= start);
    assert(start <= beginWrite());
    const void* eol = memchr(start, '\n', readableBytes());
    return static_cast<const char*>(eol);
  }

  void retrieve(size_t len) {
    assert(len <= readableBytes());
    if (len < readableBytes()) {
      readerIndex_ += len;
    } else {
      retrieveAll();
    }
  }

  void retrieveAll() {
    readerIndex_ = kCheapPrepend;
    writerIndex_ = kCheapPrepend;
  }

  void retrieveUntil(const char* end) {
    assert(peek() <= end);
    assert(end <= beginWrite());
    retrieve(end - peek());
  }

  void retrieveInt64() { retrieve(sizeof(int64_t)); }

  void retrieveInt32() { retrieve(sizeof(int32_t)); }

  void retrieveInt16() { retrieve(sizeof(int16_t)); }

  void retrieveInt8() { retrieve(sizeof(int8_t)); }

  std::string retrieveAsString(size_t len) {
    assert(len <= readableBytes());
    std::string result(peek(), len);
    retrieve(len);
    return result;
  }

  std::string retrieveAllAsString() {
    return retrieveAsString(readableBytes());
  }
  //提供一个视窗功能,相较于 peek, toStringPiece 可以兼容 string 类型
  StringPiece toStringPiece() const {
    return StringPiece(peek(), static_cast<int>(readableBytes()));
  }

  void append(const StringPiece& str) { append(str.data(), str.size()); }

  void append(const char* data, size_t len) {
    ensureWritableBytes(len);
    std::copy(data, data + len, beginWrite());
    hasWritten(len);
  }

  void append(const void* data, size_t len) {
    append(static_cast<const char*>(data), len);
  }

  void ensureWritableBytes(size_t len) {
    if (writableBytes() < len) {
      makeSpace(len);
    }
    assert(len <= writableBytes());
  }

  void hasWritten(size_t len) {
    assert(len <= writableBytes());
    writerIndex_ += len;
  }

  char* beginWrite() { return begin() + writerIndex_; }

  const char* beginWrite() const { return begin() + writerIndex_; }

  void unwrite(size_t len) {
    assert(len <= readableBytes());
    writerIndex_ -= len;
  }

  /// Append int64_t using network endian
  void appendInt64(int64_t x) {
    int64_t be64 = sockets::hostToNetwork64(x);
    append(&be64, sizeof be64);
  }

  /// Append int32_t using network endian
  void appendInt32(int32_t x) {
    int32_t be32 = sockets::hostToNetwork32(x);
    append(&be32, sizeof be32);
  }

  void appendInt16(int16_t x) {
    int16_t be16 = sockets::hostToNetwork16(x);
    append(&be16, sizeof be16);
  }

  void appendInt8(int8_t x) { append(&x, sizeof x); }

  /// Read int64_t from network endian
  /// Require: buf->readableBytes() >= sizeof(int64_t)
  int64_t readInt64() {
    int64_t result = peekInt64();
    retrieveInt64();
    return result;
  }

  /// Read int32_t from network endian
  /// Require: buf->readableBytes() >= sizeof(int32_t)
  int32_t readInt32() {
    int32_t result = peekInt32();
    retrieveInt32();
    return result;
  }

  int16_t readInt16() {
    int16_t result = peekInt16();
    retrieveInt16();
    return result;
  }

  int8_t readInt8() {
    int8_t result = peekInt8();
    retrieveInt8();
    return result;
  }

  /// Peek int64_t from network endian
  /// Require: buf->readableBytes() >= sizeof(int64_t)
  int64_t peekInt64() const {
    assert(readableBytes() >= sizeof(int64_t));
    int64_t be64 = 0;
    ::memcpy(&be64, peek(), sizeof be64);
    return sockets::networkToHost64(be64);
  }

  /// Peek int32_t from network endian
  /// Require: buf->readableBytes() >= sizeof(int32_t)
  int32_t peekInt32() const {
    assert(readableBytes() >= sizeof(int32_t));
    int32_t be32 = 0;
    ::memcpy(&be32, peek(), sizeof be32);
    return sockets::networkToHost32(be32);
  }

  int16_t peekInt16() const {
    assert(readableBytes() >= sizeof(int16_t));
    int16_t be16 = 0;
    ::memcpy(&be16, peek(), sizeof be16);
    return sockets::networkToHost16(be16);
  }

  int8_t peekInt8() const {
    assert(readableBytes() >= sizeof(int8_t));
    int8_t x = *peek();
    return x;
  }

  void prepend(const void* data, size_t len) {
    assert(len <= prependableBytes());
    readerIndex_ -= len;
    const char* d = static_cast<const char*>(data);
    std::copy(d, d + len, begin() + readerIndex_);
  }

  /// Prepend int64_t using network endian
  void prependInt64(int64_t x) {
    int64_t be64 = sockets::hostToNetwork64(x);
    prepend(&be64, sizeof be64);
  }

  /// Prepend int32_t using network endian
  void prependInt32(int32_t x) {
    int32_t be32 = sockets::hostToNetwork32(x);
    prepend(&be32, sizeof be32);
  }

  void prependInt16(int16_t x) {
    int16_t be16 = sockets::hostToNetwork16(x);
    prepend(&be16, sizeof be16);
  }

  void prependInt8(int8_t x) { prepend(&x, sizeof x); }

  void shrink(size_t reserve) {
    Buffer other;
    other.ensureWritableBytes(readableBytes() + reserve);
    other.append(toStringPiece());
    swap(other);
  }

  size_t internalCapacity() const { return buffer_.capacity(); }

  /// Read data directly into buffer.
  /// It may implement with readv(2)
  /// @return result of read(2), @c errno is saved
  ssize_t readFd(int fd, int* savedErrno);

 private:
  char* begin() {
    return &*buffer_.begin();  //将迭代器转换为 char*
  }

  const char* begin() const { return &*buffer_.begin(); }

  void makeSpace(size_t len) {
    if (writableBytes() + prependableBytes() < len + kCheapPrepend) {
      buffer_.resize(writerIndex_ + len);
    } else {
      //将可读的数据挪至缓冲区的前面，这样可以腾出空间来做新数据的写入工作
      assert(kCheapPrepend < readerIndex_);
      size_t readable = readableBytes();
      std::copy(begin() + readerIndex_, begin() + writerIndex_,
                begin() + kCheapPrepend);
      readerIndex_ = kCheapPrepend;
      writerIndex_ = readerIndex_ + readable;
      assert(readable == readableBytes());
    }
  }

 private:
  std::vector<char> buffer_;
  //使用下标而非指针的方式可以避免 buffer_ 迭代器失效
  size_t readerIndex_;
  size_t writerIndex_;
  static const char kCRLF[];
};

}  // namespace net
}  // namespace tmuduo

#endif  // TMUDUO_NET_BUFFER_H_