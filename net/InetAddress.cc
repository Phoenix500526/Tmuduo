#include "net/InetAddress.h"

#include "base/Logging.h"
#include "net/Endian.h"
#include "net/SocketsOps.h"

#include <netdb.h>
#include <netinet/in.h>

// INADDR_ANY use (type)value casting.
#pragma clang diagnostic ignored "-Wold-style-cast"
static const in_addr_t kInaddrAny = INADDR_ANY;
static const in_addr_t kInaddrLoopback = INADDR_LOOPBACK;
#pragma clang diagnostic error "-Wold-style-cast"

// Structure describing an Internet socket address.
// struct sockaddr_in {
//      sa_family_t    sin_family; /* address family: AF_INET */
//      uint16_t       sin_port;   /* port in network byte order */
//      struct in_addr sin_addr;   /* internet address */
//};

// Internet address.
// typedef uint32_t in_addr_t;
//      struct in_addr {
//      in_addr_t       s_addr;     /* address in network byte order */
// };

// struct sockaddr_in6 {
//      sa_family_t     sin6_family;   /* address family: AF_INET6 */
//      uint16_t        sin6_port;     /* port in network byte order */
//      uint32_t        sin6_flowinfo; /* IPv6 flow information */
//      struct in6_addr sin6_addr;     /* IPv6 address */
//      uint32_t        sin6_scope_id; /* IPv6 scope-id */
// };

using namespace tmuduo;
using namespace tmuduo::net;

static_assert(sizeof(InetAddress) == sizeof(struct sockaddr_in6),
              "InetAddress is same size as sockaddr_in6");
// offsetof(type, member) 判断 member 在 type 中的偏移量
static_assert(offsetof(sockaddr_in, sin_family) == 0, "sin_family offset 0");
static_assert(offsetof(sockaddr_in6, sin6_family) == 0, "sin6_family offset 0");
static_assert(offsetof(sockaddr_in, sin_port) == 2, "sin_port offset 2");
static_assert(offsetof(sockaddr_in6, sin6_port) == 2, "sin6_port offset 2");

// loopbackOnly: true 代表回环地址 127.0.0.1, false 代表任意地址,一般默认为
// 0.0.0.0
InetAddress::InetAddress(uint16_t port, bool loopbackOnly, bool ipv6) {
  static_assert(offsetof(InetAddress, addr6_) == 0, "addr6_ offset 0");
  static_assert(offsetof(InetAddress, addr_) == 0, "addr_ offset 0");
  if (ipv6) {
    memZero(&addr6_, sizeof addr6_);
    addr6_.sin6_family = AF_INET6;
    in6_addr ip = loopbackOnly ? in6addr_loopback : in6addr_any;
    addr6_.sin6_addr = ip;
    addr6_.sin6_port = sockets::hostToNetwork16(port);
  } else {
    memZero(&addr_, sizeof addr_);
    addr_.sin_family = AF_INET;
    in_addr_t ip = loopbackOnly ? kInaddrLoopback : kInaddrAny;
    addr_.sin_addr.s_addr = sockets::hostToNetwork32(ip);
    addr_.sin_port = sockets::hostToNetwork16(port);
  }
}

InetAddress::InetAddress(StringArg ip, uint16_t port, bool ipv6) {
  if (ipv6) {
    memZero(&addr6_, sizeof addr6_);
    sockets::fromIpPort(ip.c_str(), port, &addr6_);
  } else {
    memZero(&addr_, sizeof addr_);
    sockets::fromIpPort(ip.c_str(), port, &addr_);
  }
}

std::string InetAddress::toIpPort() const {
  char buf[64] = "";
  sockets::toIpPort(buf, sizeof buf, getSockAddr());
  return buf;
}

std::string InetAddress::toIp() const {
  char buf[64] = "";
  sockets::toIp(buf, sizeof buf, getSockAddr());
  return buf;
}

uint32_t InetAddress::ipNetEndian() const {
  assert(family() == AF_INET);
  return addr_.sin_addr.s_addr;
}

uint16_t InetAddress::toPort() const {
  return sockets::networkToHost16(portNetEndian());
}

static thread_local char t_resolveBuffer[64 * 1024];

//resolve 将主机名转换为 ip 地址
bool InetAddress::resolve(StringArg hostname, InetAddress* out) {
  assert(out != nullptr);
  struct hostent hent;
  struct hostent* he = nullptr;
  int herrno = 0;
  memZero(&hent, sizeof(hent));
  int ret = gethostbyname_r(hostname.c_str(), &hent, t_resolveBuffer,
                            sizeof t_resolveBuffer, &he, &herrno);
  if (ret == 0 && he != nullptr) {
    assert(he->h_addrtype == AF_INET && he->h_length == sizeof(uint32_t));
    out->addr_.sin_addr = *reinterpret_cast<struct in_addr*>(he->h_addr);
    return true;
  } else {
    if (ret) {
      LOG_SYSERR << "InetAddress::resolve";
    }
    return false;
  }
}

void InetAddress::setScopeId(uint32_t scope_id) {
  if (family() == AF_INET6) {
    addr6_.sin6_scope_id = scope_id;
  }
}