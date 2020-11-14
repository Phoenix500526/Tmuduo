#include "examples/cdns/Resolver.h"
#include "base/Logging.h"
#include "net/Channel.h"
#include "net/EventLoop.h"

#include <ares.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

using namespace tmuduo;
using namespace tmuduo::net;
using namespace cdns;

namespace {
double getSeconds(struct timeval* tv) {
  if (tv)
    return static_cast<double>(tv->tv_sec) +
           static_cast<double>(tv->tv_usec) / 1000000.0;
  else
    return -1.0;
}

const char* getSocketType(int type) {
  if (type == SOCK_DGRAM)
    return "UDP";
  else if (type == SOCK_STREAM)
    return "TCP";
  else
    return "Unknown";
}

const bool kDebug = false;
}  // namespace

Resolver::Resolver(EventLoop* loop, Option opt)
    : loop_(loop), ctx_(NULL), timerActive_(false) {
  static char lookups[] = "b";
  struct ares_options options;
  int optmask = ARES_OPT_FLAGS | ARES_OPT_SOCK_STATE_CB;
  options.flags = ARES_FLAG_NOCHECKRESP | ARES_FLAG_STAYOPEN | ARES_FLAG_IGNTC;
  options.sock_state_cb = &Resolver::ares_sock_state_callback;
  options.sock_state_cb_data = this;
  optmask |= ARES_OPT_TIMEOUT;
  options.timeout = 2;
  if (opt == Option::kDNSonly) {
    optmask |= ARES_OPT_LOOKUPS;
    options.lookups = lookups;
  }

  int status = ares_init_options(&ctx_, &options, optmask);
  if (status != ARES_SUCCESS) {
    assert(0);
  }
  ares_set_socket_callback(ctx_, &Resolver::ares_sock_create_callback, this);
}

Resolver::~Resolver() { ares_destroy(ctx_); }

bool Resolver::resolve(StringArg hostname, const Callback& cb) {
  loop_->assertInLoopThread();
  QueryData* queryData = new QueryData(this, cb);
  //ares_gethostbyname 不是线程安全的，必须保证只在 LoopThread 中使用
  ares_gethostbyname(ctx_, hostname.c_str(), AF_INET,
                     &Resolver::ares_host_callback, queryData);
  //获得 dns 的超时时间，以便设置自身的定时器
  struct timeval tv;
  struct timeval* tvp = ares_timeout(ctx_, NULL, &tv);
  double timeout = getSeconds(tvp);
  LOG_DEBUG << "timeout " << timeout << " active " << timerActive_;
  if (!timerActive_) {
    loop_->runAfter(timeout, std::bind(&Resolver::onTimer, this));
    timerActive_ = true;
  }
  return queryData != NULL;
}

void Resolver::onRead(int sockfd, Timestamp t) {
  LOG_DEBUG << "onRead " << sockfd << " at " << t.toString();
  ares_process_fd(ctx_, sockfd, ARES_SOCKET_BAD);
}

void Resolver::onTimer() {
  assert(timerActive_ == true);
  ares_process_fd(ctx_, ARES_SOCKET_BAD, ARES_SOCKET_BAD);
  struct timeval tv;
  struct timeval* tvp = ares_timeout(ctx_, NULL, &tv);
  double timeout = getSeconds(tvp);
  LOG_DEBUG << loop_->pollReturnTime().toString() << " next timeout "
            << timeout;
  if (timeout < 0) {
    timerActive_ = false;
  } else {
    loop_->runAfter(timeout, std::bind(&Resolver::onTimer, this));
  }
}

void Resolver::onQueryResult(int status, struct hostent* result,
                             const Callback& callback) {
  LOG_DEBUG << "onQueryResult " << status;
  struct sockaddr_in addr;
  memZero(&addr, sizeof addr);
  addr.sin_family = AF_INET;
  addr.sin_port = 0;
  if (result) {
    addr.sin_addr = *reinterpret_cast<in_addr*>(result->h_addr);
    if (kDebug) {
      printf("h_name %s\n", result->h_name);
      for (char** alias = result->h_aliases; *alias != NULL; ++alias) {
        printf("alias: %s\n", *alias);
      }
      for (char** haddr = result->h_addr_list; *haddr != NULL; ++haddr) {
        char buf[32];
        inet_ntop(AF_INET, *haddr, buf, sizeof buf);
        printf("  %s\n", buf);
      }
    }
  }
  InetAddress inet(addr);
  callback(inet);
}

void Resolver::onSockCreate(int sockfd, int type) {
  loop_->assertInLoopThread();
  assert(channels_.find(sockfd) == channels_.end());
  Channel* channel = new Channel(loop_, sockfd);
  channel->setReadCallback(std::bind(&Resolver::onRead, this, sockfd, _1));
  channel->enableReading();
  channels_[sockfd].reset(channel);
}

void Resolver::onSockStateChange(int sockfd, bool read, bool write) {
  loop_->assertInLoopThread();
  auto it = channels_.find(sockfd);
  assert(it != channels_.end());
  if (read) {
    //FIXME
    printf("read function has been call\n");
  } else if(write){
    //FIXME: consider in what scenario the sockfd will become writeable
    printf("wirte function has been call\n");
  }else {
    it->second->disableAll();
    it->second->remove();
    channels_.erase(it);
  }
}

void Resolver::ares_host_callback(void* data, int status, int timeout,
                                  struct hostent* hostent) {
  QueryData* query = static_cast<QueryData*>(data);
  query->owner->onQueryResult(status, hostent, query->callback);
  delete query;
}

int Resolver::ares_sock_create_callback(int sockfd, int type, void* data) {
  LOG_TRACE << "sockfd=" << sockfd << " type=" << getSocketType(type);
  static_cast<Resolver*>(data)->onSockCreate(sockfd, type);
  return 0;
}

void Resolver::ares_sock_state_callback(void* data, int sockfd, int read,
                                        int write) {
  LOG_TRACE << "sockfd = " << sockfd << " read = " << read
            << " write = " << write;
  static_cast<Resolver*>(data)->onSockStateChange(sockfd, read, write);
}