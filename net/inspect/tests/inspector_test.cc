#include "net/EventLoop.h"
#include "net/EventLoopThread.h"
#include "net/inspect/Inspector.h"

using namespace tmuduo;
using namespace tmuduo::net;

int main() {
  EventLoop loop;
  EventLoopThread t;
  Inspector ins(t.getLoopOnlyOnce(), InetAddress(12345), "inspect_test");
  loop.loop();
}