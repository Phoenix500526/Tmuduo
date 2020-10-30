#ifndef TMUDUO_NET_POLLER_POLLPOLLER_H_
#define TMUDUO_NET_POLLER_POLLPOLLER_H_

#include <vector>
#include "net/Poller.h"

struct pollfd;
namespace tmuduo {
namespace net {

class PollPoller : public Poller {
 public:
  PollPoller(EventLoop* loop);
  ~PollPoller() override;

  Timestamp poll(int timeoutMs, ChannelList* activeChannels) override;
  void updateChannel(Channel* channel) override;
  void removeChannel(Channel* channel) override;

 private:
  void fillActiveChannels(int numEvents, ChannelList* fillActiveChannels) const;
  using PollFdList = std::vector<struct pollfd>;
  PollFdList pollfds_;
};

}  // namespace net
}  // namespace tmuduo

#endif  // TMUDUO_NET_POLLER_POLLPOLLER_H_