#ifndef TMUDUO_NET_ACCEPTOR_H_
#define TMUDUO_NET_ACCEPTOR_H_

#include <functional>

#include "net/Channel.h"
#include "net/Socket.h"

namespace tmuduo {
namespace net {

class EventLoop;
class InetAddress;

// Acceptor of incoming TCP connections
class Acceptor : noncopyable {
 public:
  using NewConnectionCallback =
      std::function<void(Socket sockfd, const InetAddress&)>;
  Acceptor(EventLoop* loop, const InetAddress& listenAddr, bool reuserport);
  ~Acceptor();
  void setNewConnectionCallback(const NewConnectionCallback& cb) {
    newConnectionCallback_ = cb;
  }
  void listen();
  bool listening() const { return listening_; }

 private:
  void handleRead();
  EventLoop* loop_;
  Socket acceptSocket_;  // RAII handle
  Channel acceptChannel_;
  NewConnectionCallback newConnectionCallback_;
  bool listening_;
  int idleFd_;
};

}  // namespace net
}  // namespace tmuduo

#endif  // TMUDUO_NET_ACCEPTOR_H_