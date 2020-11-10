#ifndef TMUDUO_EXAMPLE_ASIO_CHAT_CODEC_H_
#define TMUDUO_EXAMPLE_ASIO_CHAT_CODEC_H_

#include "base/Logging.h"
#include "net/Buffer.h"
#include "net/Endian.h"
#include "net/TcpConnection.h"

class LengthHeaderCodec : tmuduo::noncopyable {
 public:
  using StringMessageCallback =
      std::function<void(const tmuduo::net::TcpConnectionPtr&,
                         const std::string& message, tmuduo::Timestamp)>;
  explicit LengthHeaderCodec(const StringMessageCallback& cb)
      : messageCallback_(cb) {}

  ~LengthHeaderCodec() = default;

  void onMessage(const tmuduo::net::TcpConnectionPtr& conn,
                 tmuduo::net::Buffer* buf, tmuduo::Timestamp receiveTime) {
    while (buf->readableBytes() >= kHeaderLen) {
      const int32_t len = buf->peekInt32();
      if (len > 65536 || len < 0) {
        LOG_ERROR << "Invalid length " << len;
        conn->shutdown();
        break;
      } else if (buf->readableBytes() >= len + kHeaderLen) {
        buf->retrieve(kHeaderLen);
        std::string message(buf->peek(), len);
        messageCallback_(conn, message, receiveTime);
        buf->retrieve(len);
      } else {
        break;
      }
    }
  }

  void send(const tmuduo::net::TcpConnectionPtr& conn,
            const tmuduo::StringPiece& message) {
    tmuduo::net::Buffer buf;
    buf.append(message.data(), message.size());
    int32_t len = static_cast<int32_t>(message.size());
    int32_t be32 = tmuduo::net::sockets::hostToNetwork32(len);
    buf.prepend(&be32, sizeof be32);
    conn->send(&buf);
  }

 private:
  StringMessageCallback messageCallback_;
  const static size_t kHeaderLen = sizeof(int32_t);
};

#endif  // TMUDUO_EXAMPLE_ASIO_CHAT_CODEC_H_