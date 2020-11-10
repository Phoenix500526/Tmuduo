#ifndef TMUDUO_EXAMPLES_IDLECONNECTION_ECHO_H_
#define TMUDUO_EXAMPLES_IDLECONNECTION_ECHO_H_

#include <boost/circular_buffer.hpp>
#include <unordered_set>
#include "net/TcpServer.h"

class EchoServer : tmuduo::noncopyable {
 public:
  EchoServer(tmuduo::net::EventLoop* loop,
             const tmuduo::net::InetAddress& listenAddr, int idleSeconds);
  ~EchoServer() = default;
  void start();

 private:
  void onConnection(const tmuduo::net::TcpConnectionPtr& conn);
  void onMessage(const tmuduo::net::TcpConnectionPtr& conn,
                 tmuduo::net::Buffer* buf, tmuduo::Timestamp time);
  void onTimer();
  void dumpConnectionBuckets() const;
  using WeakTcpConnectionPtr = std::weak_ptr<tmuduo::net::TcpConnection>;
  struct Entry : tmuduo::copyable {
    explicit Entry(const WeakTcpConnectionPtr& weakConn)
        : weakConn_(weakConn) {}
    ~Entry() {
      auto conn = weakConn_.lock();
      if (conn) {
        conn->shutdown();
      }
    }
    WeakTcpConnectionPtr weakConn_;
  };
  using EntryPtr = std::shared_ptr<Entry>;
  using WeakEntryPtr = std::weak_ptr<Entry>;
  using Bucket = std::unordered_set<EntryPtr>;
  using WeakConnectionList = boost::circular_buffer<Bucket>;
  tmuduo::net::TcpServer server_;
  WeakConnectionList connectionBuckets_;
};

#endif  // TMUDUO_EXAMPLES_IDLECONNECTION_ECHO_H_