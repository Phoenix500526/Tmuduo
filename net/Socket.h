#ifndef TMUDUO_NET_SOCKET_H_
#define TMUDUO_NET_SOCKET_H_

#include "base/noncopyable.h"

// struct tcp_info is in <netinet/tcp.h>
struct tcp_info;

namespace tmuduo {
namespace net {

class InetAddress;

// sockfd 的包装类,使用 RAII 方法管理资源,线程安全,可移动类型
class Socket : noncopyable {
 public:
  explicit Socket(int sockfd) : sockfd_(sockfd) {}
  Socket() : sockfd_(-1) {}
  Socket(Socket&& rhs) : sockfd_(rhs.sockfd_) { rhs.sockfd_ = -1; }
  Socket& operator=(Socket&& rhs);

  ~Socket();
  int fd() const { return sockfd_; }
  // return true if success
  bool getTcpInfo(struct tcp_info*) const;
  bool getTcpInfoString(char* buf, int len) const;

  // abort if address in use
  void bindAddress(const InetAddress& localaddr);
  void listen();

  /// On success, returns a non-negative integer that is
  /// a descriptor for the accepted socket, which has been
  /// set to non-blocking and close-on-exec. *peeraddr is assigned.
  /// On error, -1 is returned, and *peeraddr is untouched.
  Socket accept(InetAddress* peeraddr);

  void shutdownWrite();

  /// Enable/disable TCP_NODELAY (disable/enable Nagle's algorithm).
  void setTcpNoDelay(bool on);

  /// Enable/disable SO_REUSEADDR
  void setReuseAddr(bool on);

  /// Enable/disable SO_REUSEPORT
  void setReusePort(bool on);
  /// Enable/disable SO_KEEPALIVE
  void setKeepAlive(bool on);

 private:
  int sockfd_;
};
}  // namespace net
}  // namespace tmuduo

#endif  // TMUDUO_NET_SOCKET_H_