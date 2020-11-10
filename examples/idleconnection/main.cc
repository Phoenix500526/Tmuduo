#include <stdio.h>
#include "base/Logging.h"
#include "examples/idleconnection/echo.h"
#include "net/EventLoop.h"

using namespace tmuduo;
using namespace tmuduo::net;

int main(int argc, char* argv[]) {
  EventLoop loop;
  InetAddress listenAddr(2020);
  int idleSeconds = 10;
  if (argc > 1) {
    idleSeconds = atoi(argv[1]);
  }
  LOG_INFO << " pid = " << getpid() << ", idle seconds = " << idleSeconds;
  EchoServer server(&loop, listenAddr, idleSeconds);
  server.start();
  loop.loop();
}