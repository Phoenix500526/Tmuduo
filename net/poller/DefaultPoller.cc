#include "net/Poller.h"
#include "net/poller/EPollPoller.h"

#include <stdlib.h>

using namespace tmuduo::net;

Poller* Poller::newDefaultPoller(EventLoop* loop) {
  return new EPollPoller(loop);
}