#include "examples/asio/chat/codec.h"

#include "base/Logging.h"
#include "base/Mutex.h"
#include "net/EventLoop.h"
#include "net/TcpServer.h"

#include <stdio.h>
#include <unistd.h>
#include <set>

using namespace tmuduo;
using namespace tmuduo::net;

class ChatServer : noncopyable {
 public:
  ChatServer(EventLoop* loop, const InetAddress& addr)
      : server_(loop, addr, "ChatServer"),
        codec_(std::bind(&ChatServer::onStringMessage, this, _1, _2, _3)) {
    server_.setConnectionCallback(
        std::bind(&ChatServer::onConnection, this, _1));
    server_.setMessageCallback(
        std::bind(&LengthHeaderCodec::onMessage, &codec_, _1, _2, _3));
  }
  ~ChatServer() = default;

  void setThreadNum(int numThreads) { server_.setThreadNum(numThreads); }

  void start() { server_.start(); }

 private:
  void onConnection(const TcpConnectionPtr& conn) {
    LOG_INFO << conn->peerAddress().toIpPort() << " -> "
             << conn->localAddress().toIpPort() << " is "
             << (conn->connected() ? "UP" : "DOWN");
    UniqueLock lock(mutex_);
    if (conn->connected()) {
      connections_.insert(conn);
    } else {
      connections_.erase(conn);
    }
  }

  void onStringMessage(const TcpConnectionPtr&, const std::string& message,
                       Timestamp) {
    UniqueLock lock(mutex_);
    for (auto it = connections_.begin(); it != connections_.end(); ++it) {
      codec_.send(*it, message);
    }
  }

  using ConnectionList = std::set<TcpConnectionPtr>;
  TcpServer server_;
  mutable Mutex mutex_;
  LengthHeaderCodec codec_;
  ConnectionList connections_ GUARDED_BY(mutex_);
};

int main(int argc, char* argv[]) {
  LOG_INFO << "pid = " << getpid();
  if (argc > 1) {
    EventLoop loop;
    uint16_t port = static_cast<uint16_t>(atoi(argv[1]));
    InetAddress serverAddr(port);
    ChatServer server(&loop, serverAddr);
    if (argc > 2) {
      server.setThreadNum(atoi(argv[2]));
    }
    server.start();
    loop.loop();
  } else {
    printf("Usage: %s port\n", argv[0]);
  }
}