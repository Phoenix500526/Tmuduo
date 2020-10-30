#ifndef TMUDUO_NET_POLLER_EPOLLPOLLER_H_
#define TMUDUO_NET_POLLER_EPOLLPOLLER_H_

#include <vector>
#include "net/Poller.h"

struct epoll_event;

namespace tmuduo {
namespace net {

class EPollPoller : public Poller {
 public:
  EPollPoller(EventLoop* loop);
  ~EPollPoller() override;

  Timestamp poll(int timeoutMs, ChannelList* activeChannels) override;
  void updateChannel(Channel* channel) override;
  void removeChannel(Channel* channel) override;

 private:
  using EventList = std::vector<struct epoll_event>;
  static const int kInitEventListSize = 16;
  static const char* operationToString(int op);
  void fillActiveChannels(int numEvents, ChannelList* activeChannels) const;
  void update(int operation, Channel* channel);

  int epollfd_;
  EventList events_;
};

}  // namespace net
}  // namespace tmuduo

#endif  // TMUDUO_NET_POLLER_EPOLLPOLLER_H_