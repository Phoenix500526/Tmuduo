#include "net/Connector.h"
#include "base/Logging.h"
#include "net/EventLoop.h"
#include "net/TcpClient.h"

using namespace tmuduo;
using namespace tmuduo::net;

EventLoop* g_loop;

void connectCallback(int sockfd) {
  printf("connected.\n");
  g_loop->quit();
}

int main(void) {
  EventLoop loop;
  g_loop = &loop;
  InetAddress addr("127.0.0.1", 2020);
  ConnectorPtr connector(new Connector(&loop, addr));
  connector->setNewConnectionCallback(connectCallback);
  connector->start();
  loop.loop();
}