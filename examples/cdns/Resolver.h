#ifndef TMUDUO_EXAMPLES_CDNS_RESOLVER_H_
#define TMUDUO_EXAMPLES_CDNS_RESOLVER_H_

#include "base/StringPiece.h"
#include "base/Timestamp.h"
#include "base/noncopyable.h"
#include "net/InetAddress.h"

#include <functional>
#include <map>
#include <memory>

extern "C" {
struct hostent;
struct ares_channeldata;
typedef struct ares_channeldata* ares_channel;
}

namespace tmuduo {
namespace net {
class Channel;
class EventLoop;
}  // namespace net
}  // namespace tmuduo

namespace cdns {

class Resolver : tmuduo::noncopyable {
 public:
  using Callback = std::function<void(const tmuduo::net::InetAddress&)>;
  enum class Option {
    kDNSandHostsFile,
    kDNSonly,
  };

  explicit Resolver(tmuduo::net::EventLoop* loop,
                    Option opt = Option::kDNSandHostsFile);
  ~Resolver();
  bool resolve(tmuduo::StringArg hostname, const Callback& cb);

 private:
  struct QueryData {
    Resolver* owner;
    Callback callback;
    QueryData(Resolver* o, const Callback& cb) : owner(o), callback(cb) {}
  };
  tmuduo::net::EventLoop* loop_;
  ares_channel ctx_;
  bool timerActive_;
  using ChannelList = std::map<int, std::unique_ptr<tmuduo::net::Channel>>;
  ChannelList channels_;

  void onRead(int sockfd, tmuduo::Timestamp t);
  void onTimer();
  void onQueryResult(int status, struct hostent* result, const Callback& cb);
  void onSockCreate(int sockfd, int type);
  void onSockStateChange(int sockfd, bool read, bool write);

  static void ares_host_callback(void* data, int status, int timeous,
                                 struct hostent* hostent);
  static int ares_sock_create_callback(int sockfd, int type, void* data);
  static void ares_sock_state_callback(void* data, int sockfd, int read,
                                       int write);
};
}  // namespace cdns

#endif  // TMUDUO_EXAMPLES_CDNS_RESOLVER_H_