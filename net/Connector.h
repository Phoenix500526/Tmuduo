#ifndef TMUDUO_NET_CONNECTOR_H_
#define TMUDUO_NET_CONNECTOR_H_

#include <atomic>
#include <functional>
#include <memory>
#include "base/noncopyable.h"
#include "net/InetAddress.h"
#include "net/TimerId.h"

namespace tmuduo {
namespace net {

class Channel;
class EventLoop;
// 在网络编程当中,主动发起连接往往要比被动接受连接要麻烦一些,主要有两方面:
// 1. 错误处理麻烦
// 2. 对于某些错误,需要考虑重试.
// 而 Connector 的作用便是将错误处理的逻辑封装起来,供 TcpClient 调用
class Connector : noncopyable, public std::enable_shared_from_this<Connector> {
 public:
  using NewConnectionCallback = std::function<void(int sockfd)>;
  Connector(EventLoop* loop, const InetAddress& serverAddr);
  ~Connector();
  void setNewConnectionCallback(const NewConnectionCallback& cb) {
    newConnectionCallback_ = cb;
  }
  // can be called in any thread
  void start();
  void stop();

  // must be called in loop thread
  void restart();

  const InetAddress& serverAddr() const { return serverAddr_; }

 private:
  enum class States { kDisconnected, kConnecting, kConnected };
  static const int kMaxRetryDelayMs = 30 * 1000;
  static const int kInitRetryDelayMs = 500;

  void setState(States s) { state_.store(s); }
  const char* stateToString(States s) const;
  void startInLoop();
  void stopInLoop();
  void connect();
  void connecting(int sockfd);
  void handleWrite();
  void handleError();
  void retry(int sockfd);
  int removeAndResetChannel();
  void resetChannel();

  EventLoop* loop_;
  TimerId timerId;
  InetAddress serverAddr_;
  bool connect_;
  std::atomic<States> state_;  //使用atomic
  std::unique_ptr<Channel> channel_;
  NewConnectionCallback newConnectionCallback_;
  int retryDelayMs_;
};

}  // namespace net
}  // namespace tmuduo

#endif  // TMUDUO_NET_CONNECTOR_H_