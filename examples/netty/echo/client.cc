#include "base/Logging.h"
#include "base/Thread.h"
#include "net/EventLoop.h"
#include "net/InetAddress.h"
#include "net/TcpClient.h"

#include <stdio.h>
#include <unistd.h>
#include <utility>

using namespace tmuduo;
using namespace tmuduo::net;
using std::string;

class EchoClient : noncopyable {
 public:
  EchoClient(EventLoop* loop, const InetAddress& listenAddr, int size)
      : loop_(loop),
        client_(loop, listenAddr, "EchoClient"),
        message_(size, 'H') {
    client_.setConnectionCallback(
        std::bind(&EchoClient::onConnection, this, _1));
    client_.setMessageCallback(
        std::bind(&EchoClient::onMessage, this, _1, _2, _3));
  }
  ~EchoClient() = default;
  void connect() { client_.connect(); }

 private:
  void onConnection(const TcpConnectionPtr& conn) {
    LOG_TRACE << conn->localAddress().toIpPort() << " -> "
              << conn->peerAddress().toIpPort() << " is "
              << (conn->connected() ? "UP" : "DOWN");
    if (conn->connected()) {
      conn->setTcpNoDelay(true);
      conn->send(message_);
    } else {
      loop_->quit();
    }
  }

  void onMessage(const TcpConnectionPtr& conn, Buffer* buf, Timestamp) {
    conn->send(buf);
  }
  EventLoop* loop_;
  TcpClient client_;
  string message_;
};

int main(int argc, char* argv[]) {
  LOG_INFO << "pid = " << getpid() << ", tid = " << CurrentThread::tid();
  if (argc > 1) {
    EventLoop loop;
    InetAddress serverAddr(argv[1], 2020);
    int size = 256;
    if (argc > 2) {
      size = atoi(argv[2]);
    }
    EchoClient client(&loop, serverAddr, size);
    client.connect();
    loop.loop();
  } else {
    printf("Usage: %s host_ip [msg_size]\n", argv[0]);
  }
}