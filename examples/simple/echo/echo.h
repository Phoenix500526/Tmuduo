#ifndef TMUDUO_EXAMPLES_SIMPLE_ECHO_ECHO_H_
#define TMUDUO_EXAMPLES_SIMPLE_ECHO_ECHO_H_

#include "net/TcpServer.h"

class EchoServer {
 public:
  EchoServer(tmuduo::net::EventLoop* loop,
             const tmuduo::net::InetAddress& listenAddr);
  void start();
  ~EchoServer() = default;

 private:
  void onConnection(const tmuduo::net::TcpConnectionPtr& conn);
  void onMessage(const tmuduo::net::TcpConnectionPtr& conn,
                 tmuduo::net::Buffer* buf, tmuduo::Timestamp time);
  tmuduo::net::TcpServer server_;
};

#endif  // TMUDUO_EXAMPLES_SIMPLE_ECHO_ECHO_H_