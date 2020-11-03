#ifndef TMUDUO_NET_INSPECT_INSPECTOR_H_
#define TMUDUO_NET_INSPECT_INSPECTOR_H_

#include "base/Mutex.h"
#include "net/http/HttpRequest.h"
#include "net/http/HttpServer.h"

#include <map>

namespace tmuduo {
namespace net {

class ProcessInspector;
class PerformanceInspector;
class SystemInspector;

// An internal inspector of the running process, usually a singleton.
// Better to run in a seperated thread, as some method may block for seconds
class Inspector : noncopyable {
 public:
  using ArgList = std::vector<std::string>;
  using Callback =
      std::function<std::string(HttpRequest::Method, const ArgList& arg)>;
  Inspector(EventLoop* loop, const InetAddress& httpAddr,
            const std::string& name);
  ~Inspector();

  // Add a Callback for handling the special uri : /module/command
  void add(const std::string& module, const std::string& command,
           const Callback& cb, const std::string& help);
  void remove(const std::string& module, const std::string& command);

 private:
  using CommandList = std::map<std::string, Callback>;
  using HelpList = std::map<std::string, std::string>;

  void start();
  void onRequest(const HttpRequest& req, HttpResponse* resp);

  HttpServer server_;
  std::unique_ptr<ProcessInspector> processInspector_;
  std::unique_ptr<PerformanceInspector> performanceInspector_;
  std::unique_ptr<SystemInspector> systemInspector_;
  Mutex mutex_;
  std::map<std::string, CommandList> modules_ GUARDED_BY(mutex_);
  std::map<std::string, HelpList> helps_ GUARDED_BY(mutex_);
};

}  // namespace net
}  // namespace tmuduo

#endif  // TMUDUO_NET_INSPECT_INSPECTOR_H_