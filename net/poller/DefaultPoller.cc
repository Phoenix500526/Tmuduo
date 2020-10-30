#include <stdlib.h>
#include "net/Poller.h"
#include "net/poller/EPollPoller.h"
#include "net/poller/PollPoller.h"

using namespace tmuduo::net;

Poller* Poller::newDefaultPoller(EventLoop* loop) {
  if (::getenv("TMUDUO_USE_POLL")) {
    return new PollPoller(loop);
  } else {
    return new EPollPoller(loop);
  }
}