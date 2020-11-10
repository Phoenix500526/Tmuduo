#include "base/Logging.h"
#include "base/Thread.h"
#include "net/EventLoop.h"
#include "net/InetAddress.h"
#include "net/TcpServer.h"

#include <stdio.h>

using namespace tmuduo;
using namespace tmuduo::net;
using std::string;

void onConnection(const TcpConnectionPtr& conn) {
  if (conn->connected()) {
    conn->setTcpNoDelay(true);
  }
}

void onMessage(const TcpConnectionPtr& conn, Buffer* buf, Timestamp) {
  conn->send(buf);
}

int main(int argc, char* argv[]) {
  if (argc > 3) {
    LOG_INFO << "pid = " << getpid() << ", tid = " << CurrentThread::tid();
    const char* ip = argv[1];
    uint16_t port = static_cast<uint16_t>(atoi(argv[2]));
    InetAddress listenAddr(ip, port);
    int thread_num = atoi(argv[3]);
    EventLoop loop;
    TcpServer server(&loop, listenAddr, "PingPong");
    server.setConnectionCallback(onConnection);
    server.setMessageCallback(onMessage);
    if (thread_num > 1) {
      server.setThreadNum(thread_num);
    }
    server.start();
    loop.loop();
  } else {
    printf("Usage: %s <address> <port> <threads>\n", argv[0]);
  }
}