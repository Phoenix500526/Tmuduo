#ifndef TMUDUO_NET_INETADDRESS_H_
#define TMUDUO_NET_INETADDRESS_H_

#include "base/StringPiece.h"
#include "base/copyable.h"

#include <netinet/in.h>

namespace tmuduo {

namespace net {

namespace sockets {

const struct sockaddr* sockaddr_cast(const struct sockaddr_in6* addr);

}  // namespace sockets

// Wrapper of sockaddr_in, a POD interface class
class InetAddress : public tmuduo::copyable {
 public:
  explicit InetAddress(uint16_t port = 0, bool loopbackOnly = false,
                       bool ipv6 = false);
  InetAddress(StringArg ip, uint16_t port, bool ipv6 = false);
  explicit InetAddress(const struct sockaddr_in& addr) : addr_(addr) {}
  explicit InetAddress(const struct sockaddr_in6& addr) : addr6_(addr) {}
  sa_family_t family() const { return addr_.sin_family; }
  std::string toIp() const;
  std::string toIpPort() const;
  uint16_t toPort() const;
  const struct sockaddr* getSockAddr() const {
    return sockets::sockaddr_cast(&addr6_);
  }
  void setSockAddrInet6(const struct sockaddr_in6& addr6) { addr6_ = addr6; }

  uint32_t ipNetEndian() const;
  uint16_t portNetEndian() const { return addr_.sin_port; }

  // thread safe
  static bool resolve(StringArg hostname, InetAddress* result);

  // set IPv6 ScopeID
  void setScopeId(uint32_t scope_id);
  ~InetAddress() = default;

 private:
  union {
    struct sockaddr_in addr_;
    struct sockaddr_in6 addr6_;
  };
};

}  // namespace net

}  // namespace tmuduo

#endif  // TMUDUO_NET_INETADDRESS_H_