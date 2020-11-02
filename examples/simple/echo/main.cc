#include "examples/simple/echo/echo.h"

#include "base/Logging.h"
#include "net/EventLoop.h"

#include <unistd.h>

using namespace tmuduo;

int main(void) {
  LOG_INFO << "pid = " << getpid();
  net::EventLoop loop;
  net::InetAddress listenAddr(2020);
  EchoServer server(&loop, listenAddr);
  server.start();
  loop.loop();
  return 0;
}