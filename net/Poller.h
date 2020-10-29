#ifndef TMUDUO_NET_POLLER_H_
#define TMUDUO_NET_POLLER_H_

#include <map>
#include <vector>

#include "base/Timestamp.h"
#include "net/EventLoop.h"

namespace tmuduo {
namespace net {

class Channel;

// IO 复用的基类
class Poller : noncopyable {
 public:
  using ChannelList = std::vector<Channel*>;
  Poller(EventLoop* loop);
  virtual ~Poller() = default;
  //必须在 loop 线程当中被调用
  virtual Timestamp poll(int timeoutMs, ChannelList* activeChannels) = 0;

  //必须在 loop 线程当中被调用
  virtual void updateChannel(Channel* channel) = 0;

  //必须在 loop 线程当中被调用,用于在析构过程中释放通道
  virtual void removeChannel(Channel* channel) = 0;

  virtual bool hasChannel(Channel* channel) const;

  static Poller* newDefaultPoller(EventLoop* loop);

  void assertInLoopThread() const { ownerLoop_->assertInLoopThread(); }

 protected:
  using ChannelMap = std::map<int, Channel*>;
  ChannelMap channels_;

 private:
  EventLoop* ownerLoop_;
};

}  // namespace net
}  // namespace tmuduo

#endif  // TMUDUO_NET_POLLER_H_