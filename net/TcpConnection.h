#ifndef TMUDUO_NET_TCPCONNECTION_H_
#define TMUDUO_NET_TCPCONNECTION_H_

#include "base/StringPiece.h"
#include "base/TypeCast.h"
#include "base/noncopyable.h"
#include "net/Buffer.h"
#include "net/Callbacks.h"
#include "net/InetAddress.h"

#include <boost/any.hpp>

// struct tcp_info is in <netinet/tcp.h>
struct tcp_info;

namespace tmuduo {
namespace net {

class Channel;
class EventLoop;
class Socket;

class TcpConnection : noncopyable,
                      public std::enable_shared_from_this<TcpConnection> {
 public:
  // TcpConnection 并不是提供给用户创建的
  TcpConnection(EventLoop* loop, const std::string& name, int sockfd,
                const InetAddress& localAddr, const InetAddress& peerAddr);
  ~TcpConnection();

  EventLoop* getLoop() const { return loop_; }
  const std::string& name() const { return name_; }
  const InetAddress& localAddress() const { return localAddr_; }
  const InetAddress& peerAddress() const { return peerAddr_; }
  bool connected() const { return state_ == StateE::kConnected; }
  bool disconnected() const { return state_ == StateE::kDisconnected; }

  // return true if success.
  bool getTcpInfo(struct tcp_info*) const;
  std::string getTcpInfoString() const;

  // send 相关函数
  // void send(std::string&& message);
  void send(const void* message, int len);
  void send(const StringPiece& message);
  void send(Buffer* message);  // this one will swap data
  // void send(Buffer&& message);

  // reading or not
  void startRead();
  void stopRead();
  bool isReading() const {
    return reading_;
  };  // NOT thread safe, may race with start/stopReadInLoop

  void setContext(const boost::any& context) { context_ = context; }

  const boost::any& getContext() const { return context_; }

  boost::any* getMutableContext() { return &context_; }

  void setConnectionCallback(const ConnectionCallback& cb) {
    connectionCallback_ = cb;
  }
  void setMessageCallback(const MessageCallback& cb) { messageCallback_ = cb; }
  void setWriteCompleteCallback(const WriteCompleteCallback& cb) {
    writeCompleteCallback_ = cb;
  }
  void setHighWaterMarkCallback(const HighWaterMarkCallback& cb,
                                size_t highWaterMark) {
    highWaterMarkCallback_ = cb;
    highWaterMark_ = highWaterMark;
  }
  // closeCallback_ 主要是提供给 TcpServer 和 TcpClient 使用的,用来
  // 通知他们移除所持有的 TcpConnectionPtr 的,而不是提供给普通用户使用的
  void setCloseCallback(const CloseCallback& cb) { closeCallback_ = cb; }

  // Advanced interface
  Buffer* inputBuffer() { return &inputBuffer_; }
  Buffer* outputBuffer() { return &outputBuffer_; }

  void shutdown();  // NOT thread safe, no simultaneous calling
  void forceClose();
  void forceCloseWithDelay(double seconds);
  void setTcpNoDelay(bool on);

  // called when TcpServer accepts a new connection
  void connectEstablished();  // should be called only once
  // called when TcpServer has removed me from its map
  void connectDestroyed();  // should be called only once

 private:
  enum class StateE {
    kDisconnected,
    kConnecting,
    kConnected,
    kDisconnecting,
  };
  void setState(StateE s) { state_ = s; }
  const char* stateToString() const;
  //处理函数
  void handleRead(Timestamp receiveTime);
  void handleWrite();
  void handleClose();
  void handleError();

  void sendInLoop(const StringPiece& message);
  void sendInLoop(const void* message, size_t len);
  // void sendInLoop(std::string&& message);

  void shutdownInLoop();
  void forceCloseInLoop();

  void startReadInLoop();
  void stopReadInLoop();

 private:
  EventLoop* loop_;
  const std::string name_;
  StateE state_;  // FIXME: use atomic variable
  bool reading_;

  std::unique_ptr<Socket> socket_;
  std::unique_ptr<Channel> channel_;
  const InetAddress localAddr_;
  const InetAddress peerAddr_;
  ConnectionCallback connectionCallback_;
  MessageCallback messageCallback_;
  WriteCompleteCallback writeCompleteCallback_;
  HighWaterMarkCallback highWaterMarkCallback_;
  CloseCallback closeCallback_;
  size_t highWaterMark_;
  Buffer inputBuffer_;
  Buffer outputBuffer_;
  boost::any context_;
};

//由于用户也可以持有 TcpConnection,因此采用 shared_ptr 来保存
using TcpConnectionPtr = std::shared_ptr<TcpConnection>;

}  // namespace net
}  // namespace tmuduo

#endif  // TMUDUO_NET_TCPCONNECTION_H_