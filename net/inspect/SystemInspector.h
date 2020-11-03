#ifndef TMUDUO_NET_INSPECT_SYSTEMINSPECTOR_H_
#define TMUDUO_NET_INSPECT_SYSTEMINSPECTOR_H_

#include "net/inspect/Inspector.h"

namespace tmuduo {
namespace net {

class SystemInspector : noncopyable {
 public:
  void registerCommands(Inspector* ins);
  static std::string overview(HttpRequest::Method, const Inspector::ArgList&);
  static std::string loadavg(HttpRequest::Method, const Inspector::ArgList&);
  static std::string version(HttpRequest::Method, const Inspector::ArgList&);
  static std::string cpuinfo(HttpRequest::Method, const Inspector::ArgList&);
  static std::string meminfo(HttpRequest::Method, const Inspector::ArgList&);
  static std::string stat(HttpRequest::Method, const Inspector::ArgList&);
};

}  // namespace net
}  // namespace tmuduo

#endif  // TMUDUO_NET_INSPECT_SYSTEMINSPECTOR_H_