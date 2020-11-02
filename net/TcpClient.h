#ifndef TMUDUO_NET_TCPCLIENT_H_
#define TMUDUO_NET_TCPCLIENT_H_

#include "base/Mutex.h"
#include "net/TcpConnection.h"

namespace tmuduo {
namespace net {

class Connector;
using ConnectorPtr = std::shared_ptr<Connector>;

class TcpClient : noncopyable {
 public:
  TcpClient(EventLoop* loop, const InetAddress& serverAddr,
            const std::string& nameArg);
  ~TcpClient();
  void connect();
  void disconnect();
  void stop();

  TcpConnectionPtr connection() const {
    UniqueLock lock(mutex_);
    return connection_;
  }

  EventLoop* getLoop() const { return loop_; }
  bool retry() const { return retry_; }
  void enableRetry() { retry_ = true; }
  const std::string& name() const { return name_; }
  // Not thread safe.
  void setConnectionCallback(ConnectionCallback cb) {
    connectionCallback_ = std::move(cb);
  }

  // Not thread safe.
  void setMessageCallback(MessageCallback cb) {
    messageCallback_ = std::move(cb);
  }

  // Not thread safe.
  void setWriteCompleteCallback(WriteCompleteCallback cb) {
    writeCompleteCallback_ = std::move(cb);
  }

 private:
  // Not thread safe, but in loop
  void newConnection(int sockfd);
  // Not thread safe, but in loop
  void removeConnection(const TcpConnectionPtr& conn);

  EventLoop* loop_;
  ConnectorPtr connector_;
  const std::string name_;
  ConnectionCallback connectionCallback_;
  MessageCallback messageCallback_;
  WriteCompleteCallback writeCompleteCallback_;
  bool retry_;
  bool connect_;
  int nextConnId_;  // always in loop thread
  mutable Mutex mutex_;
  TcpConnectionPtr connection_ GUARDED_BY(mutex_);
};

}  // namespace net
}  // namespace tmuduo

#endif  // TMUDUO_NET_TCPCLIENT_H_