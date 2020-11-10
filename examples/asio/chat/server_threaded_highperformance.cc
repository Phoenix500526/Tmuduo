#include "base/Logging.h"
#include "base/Mutex.h"
#include "base/ThreadLocalSingleton.h"
#include "examples/asio/chat/codec.h"
#include "net/EventLoop.h"
#include "net/TcpServer.h"

#include <stdio.h>
#include <unistd.h>
#include <set>

using namespace tmuduo;
using namespace tmuduo::net;
using std::string;

class ChatServer {
 public:
  ChatServer(EventLoop* loop, const InetAddress& listenAddr)
      : server_(loop, listenAddr, "ChatServer"),
        codec_(std::bind(&ChatServer::onStringMessage, this, _1, _2, _3)) {
    server_.setConnectionCallback(
        std::bind(&ChatServer::onConnection, this, _1));
    server_.setMessageCallback(
        std::bind(&LengthHeaderCodec::onMessage, &codec_, _1, _2, _3));
  }
  ~ChatServer() = default;

  void setThreadNum(int numThreads) { server_.setThreadNum(numThreads); }

  void start() {
    server_.setThreadInitCallback(std::bind(&ChatServer::threadInit, this, _1));
    server_.start();
  }

 private:
  using ConnectionList = std::set<TcpConnectionPtr>;
  using LocalConnections = ThreadLocalSingleton<ConnectionList>;
  void onConnection(const TcpConnectionPtr& conn) {
    LOG_INFO << conn->peerAddress().toIpPort() << " -> "
             << conn->localAddress().toIpPort() << " is "
             << (conn->connected() ? "UP" : "DOWN");
    if (conn->connected()) {
      LocalConnections::instance().insert(conn);
    } else {
      LocalConnections::instance().erase(conn);
    }
  }

  void onStringMessage(const TcpConnectionPtr&, const string& message,
                       Timestamp) {
    EventLoop::Functor f =
        std::bind(&ChatServer::distributeMessage, this, message);
    LOG_DEBUG;
    UniqueLock lock(mutex_);
    for (auto it = loops_.begin(); it != loops_.end(); ++it) {
      (*it)->queueInLoop(f);
    }
    LOG_DEBUG;
  }

  void distributeMessage(const string& message) {
    LOG_DEBUG << "begin";
    for (auto it = LocalConnections::instance().begin();
         it != LocalConnections::instance().end(); ++it) {
      codec_.send(*it, message);
    }
    LOG_DEBUG << "end";
  }

  void threadInit(EventLoop* loop) {
    LocalConnections::instance();
    UniqueLock lock(mutex_);
    loops_.insert(loop);
  }

  mutable Mutex mutex_;
  TcpServer server_;
  LengthHeaderCodec codec_;
  std::set<EventLoop*> loops_ GUARDED_BY(mutex_);
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