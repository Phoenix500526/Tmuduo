#ifndef TMUDUO_NET_INSPECT_PROCESSINSPECTOR_H_
#define TMUDUO_NET_INSPECT_PROCESSINSPECTOR_H_

#include "net/inspect/Inspector.h"

namespace tmuduo {
namespace net {

class ProcessInspector : noncopyable {
 public:
  void registerCommands(Inspector* ins);
  static std::string overview(HttpRequest::Method, const Inspector::ArgList&);
  static std::string pid(HttpRequest::Method, const Inspector::ArgList&);
  static std::string procStatus(HttpRequest::Method, const Inspector::ArgList&);
  static std::string openedFiles(HttpRequest::Method,
                                 const Inspector::ArgList&);
  static std::string threads(HttpRequest::Method, const Inspector::ArgList&);

  static std::string username_;
};

}  // namespace net
}  // namespace tmuduo

#endif  // TMUDUO_NET_INSPECT_PROCESSINSPECTOR_H_