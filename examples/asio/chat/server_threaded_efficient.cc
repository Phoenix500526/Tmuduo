#include "base/Logging.h"
#include "base/Mutex.h"
#include "examples/asio/chat/codec.h"
#include "net/EventLoop.h"
#include "net/TcpServer.h"

#include <stdio.h>
#include <unistd.h>
#include <set>

using namespace tmuduo;
using namespace tmuduo::net;

class ChatServer : noncopyable {
 public:
  ChatServer(EventLoop* loop, const InetAddress& listenAddr)
      : server_(loop, listenAddr, "ChatServer"),
        codec_(std::bind(&ChatServer::onStringMessage, this, _1, _2, _3)),
        connections_(new ConnectionList) {
    server_.setConnectionCallback(
        std::bind(&ChatServer::onConnection, this, _1));
    server_.setMessageCallback(
        std::bind(&LengthHeaderCodec::onMessage, &codec_, _1, _2, _3));
  }
  ~ChatServer() = default;

  void setThreadNum(int threadnum) { server_.setThreadNum(threadnum); }

  void start() { server_.start(); }

 private:
  void onConnection(const TcpConnectionPtr& conn) {
    LOG_INFO << conn->peerAddress().toIpPort() << " -> "
             << conn->localAddress().toIpPort() << " is "
             << (conn->connected() ? "UP" : "DOWN");
    UniqueLock lock(mutex_);
    if (!connections_.unique()) {
      connections_.reset(new ConnectionList(*connections_));
    }
    assert(connections_.unique());
    if (conn->connected()) {
      connections_->insert(conn);
    } else {
      connections_->erase(conn);
    }
  }

  void onStringMessage(const TcpConnectionPtr&, const std::string& message,
                       Timestamp) {
    ConnectionListPtr connections = getConnectionList();
    for (auto it = connections->begin(); it != connections->end(); ++it) {
      codec_.send(*it, message);
    }
  }

  using ConnectionList = std::set<TcpConnectionPtr>;
  using ConnectionListPtr = std::shared_ptr<ConnectionList>;

  ConnectionListPtr getConnectionList() {
    UniqueLock lock(mutex_);
    return connections_;
  }

  TcpServer server_;
  LengthHeaderCodec codec_;
  mutable Mutex mutex_;
  ConnectionListPtr connections_ GUARDED_BY(mutex_);
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
    printf("Usage: %s port [thread_num]\n", argv[0]);
  }
}