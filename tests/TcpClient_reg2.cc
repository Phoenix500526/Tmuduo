#include "net/TcpClient.h"
#include "base/Logging.h"
#include "base/Thread.h"
#include "net/EventLoop.h"

using namespace tmuduo;
using namespace tmuduo::net;

void threadFunc(EventLoop* loop) {
  InetAddress serverAddr("127.0.0.1", 2020);
  TcpClient client(loop, serverAddr, "TcpClient");
  client.connect();
  CurrentThread::sleepUsec(1000 * 1000);
  // client desturcts when connected;
}

int main(void) {
  Logger::setLogLevel(Logger::DEBUG);
  EventLoop loop;
  loop.runAfter(3.0, std::bind(&EventLoop::quit, &loop));
  Thread thr(std::bind(threadFunc, &loop));
  loop.loop();
}