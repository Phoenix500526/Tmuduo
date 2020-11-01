#ifndef TMUDUO_NET_TCPSERVER_H_
#define TMUDUO_NET_TCPSERVER_H_

#include "base/TypeCast.h"
#include "net/TcpConnection.h"

#include <map>

namespace tmuduo {
namespace net {

class Acceptor;
class EventLoop;
class EventLoopThreadPool;

class TcpServer : noncopyable {
 public:
  using ThreadInitCallback = std::function<void(EventLoop*)>;
  enum class Option {
    kNoReusePort,
    kReusePort,
  };
  TcpServer(EventLoop* loop, const InetAddress& listenAddr,
            const std::string& nameArg, Option option = Option::kNoReusePort);
  ~TcpServer();
  const std::string& ipPort() const { return ipPort_; }
  const std::string& name() const { return name_; }
  EventLoop* getLoop() const { return loop_; }
  /// Set the number of threads for handling input.
  ///
  /// Always accepts new connection in loop's thread.
  /// Must be called before @c start
  /// @param numThreads
  /// - 0 means all I/O in loop's thread, no thread will created.
  ///   this is the default value.
  /// - 1 means all I/O in another thread.
  /// - N means a thread pool with N threads, new connections
  ///   are assigned on a round-robin basis.
  void setThreadNum(int numThreads);
  void setThreadInitCallback(const ThreadInitCallback& cb) {
    threadInitCallback_ = cb;
  }
  //只有在调用 start 之后才生效
  std::shared_ptr<EventLoopThreadPool> threadPool() { return threadPool_; }

  // thread safe
  void start();

  // Not thread safe
  void setConnectionCallback(const ConnectionCallback& cb) {
    connectionCallback_ = cb;
  }

  // Not thread safe
  void setMessageCallback(const MessageCallback& cb) { messageCallback_ = cb; }

  // Not thread safe
  void setWriteCompleteCallback(const WriteCompleteCallback& cb) {
    writeCompleteCallback_ = cb;
  }

 private:
  using ConnectionMap = std::map<std::string, TcpConnectionPtr>;
  //只在 loop 中执行,不必是 thread safe 的
  void newConnection(int sockfd, const InetAddress& peerAddr);

  // thread safe
  void removeConnection(const TcpConnectionPtr& conn);

  /// Not thread safe, but in loop
  void removeConnectionInLoop(const TcpConnectionPtr& conn);

  EventLoop* loop_;
  const std::string ipPort_;
  const std::string name_;
  std::unique_ptr<Acceptor> acceptor_;
  std::shared_ptr<EventLoopThreadPool> threadPool_;
  ConnectionCallback connectionCallback_;
  MessageCallback messageCallback_;
  WriteCompleteCallback writeCompleteCallback_;
  ThreadInitCallback threadInitCallback_;
  std::atomic<std::int32_t> started_;
  // nextConnId_ 总在 loop thread 中执行
  int nextConnId_;
  ConnectionMap connections_;
};

}  // namespace net
}  // namespace tmuduo

#endif  // TMUDUO_NET_TCPSERVER_H_