#include "net/TcpClient.h"
#include "base/Logging.h"
#include "net/EventLoopThread.h"

using namespace tmuduo;
using namespace tmuduo::net;

int main(int argc, char* argv[]) {
  Logger::setLogLevel(Logger::DEBUG);
  EventLoopThread loopThread;
  {
    InetAddress serverAddr("127.0.0.1", 2020);
    TcpClient client(loopThread.getLoopOnlyOnce(), serverAddr, "TcpClient");
    client.connect();
    CurrentThread::sleepUsec(500 * 1000);
    client.disconnect();
  }
  CurrentThread::sleepUsec(1000 * 1000);
}