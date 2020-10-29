#ifndef TMUDUO_NET_CHANNEL_H_
#define TMUDUO_NET_CHANNEL_H_

#include "base/Timestamp.h"
#include "base/noncopyable.h"

#include <functional>
#include <memory>

namespace tmuduo {

namespace net {

class EventLoop;

// A selectable I/O channel
// Channel 并不占据任何形式的文件描述符,包括了 socket, eventfd, timerfd 以及
// signalfd. Channel 对用户透明
class Channel : noncopyable {
 public:
  using EventCallback = std::function<void()>;
  using ReadEventCallback = std::function<void(Timestamp)>;
  Channel(EventLoop* loop, int fd);
  ~Channel();
  //事件处理函数
  void handleEvent(Timestamp receiveTime);

  void setReadCallback(ReadEventCallback cb) { readCallback_ = std::move(cb); }
  void setWriteCallback(EventCallback cb) { writeCallback_ = std::move(cb); }
  void setCloseCallback(EventCallback cb) { closeCallback_ = std::move(cb); }
  void setErrorCallback(EventCallback cb) { errorCallback_ = std::move(cb); }

  //将通道和通道的拥有者绑定起来,通道的拥有者的生命周期由 shared_ptr 来管理
  //避免在处理事件的过程当中遭到销毁
  void tie(const std::shared_ptr<void>&);

  int fd() const { return fd_; }
  int events() const { return events_; }
  void set_revents(int revt) { revents_ = revt; }  // used by pollers
  bool isNoneEvent() const { return kNoneEvent == events_; }
  void enableReading() {
    events_ |= kReadEvent;
    update();
  }
  void disableReading() {
    events_ &= ~kReadEvent;
    update();
  }
  void enableWriting() {
    events_ |= kWriteEvent;
    update();
  }
  void disableWriting() {
    events_ &= ~kWriteEvent;
    update();
  }
  void disableAll() {
    events_ = kNoneEvent;
    update();
  }
  bool isWriting() const { return events_ & kWriteEvent; }
  bool isReading() const { return events_ & kReadEvent; }

  // for Poller
  int index() { return index_; }
  void set_index(int idx) { index_ = idx; }

  // for debug
  std::string reventsToString() const;
  std::string eventsToString() const;

  void doNotLogHup() { logHup_ = false; }
  EventLoop* ownerLoop() { return loop_; }
  void remove();

 private:
  static std::string eventsToString(int fd, int ev);

  void update();
  void handleEventWithGuard(Timestamp receiveTime);

  static const int kNoneEvent;
  static const int kReadEvent;
  static const int kWriteEvent;

  EventLoop* loop_;
  const int fd_;
  int events_;
  int revents_;
  int index_;
  bool logHup_;
  //使用 weak_ptr,在 handle event 的时候能够判断使用通道的对象是否被销毁
  std::weak_ptr<void> tie_;
  bool tied_;
  bool eventHandling_;
  bool addedToLoop_;
  ReadEventCallback readCallback_;
  EventCallback writeCallback_;
  EventCallback closeCallback_;
  EventCallback errorCallback_;
};

}  // namespace net

}  // namespace tmuduo

#endif  // TMUDUO_NET_CHANNEL_H_