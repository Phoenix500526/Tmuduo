#ifndef TMUDUO_NET_INSPECT_PERFORMANCEINSPECTOR_H_
#define TMUDUO_NET_INSPECT_PERFORMANCEINSPECTOR_H_

#include "net/inspect/Inspector.h"

namespace tmuduo {
namespace net {

class PerformanceInspector : noncopyable {
 public:
  void registerCommands(Inspector* ins);

  static std::string heap(HttpRequest::Method, const Inspector::ArgList&);
  static std::string growth(HttpRequest::Method, const Inspector::ArgList&);
  static std::string profile(HttpRequest::Method, const Inspector::ArgList&);
  static std::string cmdline(HttpRequest::Method, const Inspector::ArgList&);
  static std::string memstats(HttpRequest::Method, const Inspector::ArgList&);
  static std::string memhistogram(HttpRequest::Method,
                                  const Inspector::ArgList&);
  static std::string releaseFreeMemory(HttpRequest::Method,
                                       const Inspector::ArgList&);
  static std::string symbol(HttpRequest::Method, const Inspector::ArgList&);
};

}  // namespace net
}  // namespace tmuduo

#endif  // TMUDUO_NET_INSPECT_PERFORMANCEINSPECTOR_H_