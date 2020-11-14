#include <stdio.h>
#include "examples/cdns/Resolver.h"
#include "net/EventLoop.h"

using namespace tmuduo;
using namespace tmuduo::net;
using namespace cdns;
using std::string;

EventLoop* g_loop;
int count = 0;
int total = 0;

void quit() { g_loop->quit(); }

void resolveCallback(const string& host, const InetAddress& addr) {
  printf("resolveCallback %s -> %s\n", host.c_str(), addr.toIpPort().c_str());
  if (++count == total) quit();
}

void resolve(Resolver* res, const string& host) {
  res->resolve(host, std::bind(&resolveCallback, host, _1));
}

int main(int argc, char* argv[]) {
  EventLoop loop;
  loop.runAfter(10, quit);
  g_loop = &loop;
  Resolver resolver(&loop, argc == 1 ? Resolver::Option::kDNSonly
                                     : Resolver::Option::kDNSandHostsFile);
  if (argc == 1) {
    total = 3;
    resolve(&resolver, "www.hacker-cube.com");
    resolve(&resolver, "www.baidu.com");
    resolve(&resolver, "www.jianshu.com");
  } else {
    total = argc - 1;
    for (int i = 1; i < argc; ++i) {
      resolve(&resolver, argv[i]);
    }
  }
  loop.loop();
}