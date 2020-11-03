#ifndef TMUDUO_NET_HTTP_HTTPSERVER_H_
#define TMUDUO_NET_HTTP_HTTPSERVER_H_

#include "net/TcpServer.h"

namespace tmuduo {
namespace net {

class HttpRequest;
class HttpResponse;

class HttpServer : noncopyable {
 public:
  using HttpCallback = std::function<void(const HttpRequest&, HttpResponse*)>;
  HttpServer(EventLoop* loop, const InetAddress& listenAddr,
             const std::string& name,
             TcpServer::Option option = TcpServer::Option::kNoReusePort);

  ~HttpServer() = default;

  EventLoop* getLoop() const { return server_.getLoop(); }

  void setHttpCallback(const HttpCallback& cb) { httpCallback_ = cb; }

  void setThreadNum(int numThreads) { server_.setThreadNum(numThreads); }

  void start();

 private:
  void onConnection(const TcpConnectionPtr& conn);
  void onMessage(const TcpConnectionPtr& conn, Buffer* buf,
                 Timestamp receiveTime);
  void onRequest(const TcpConnectionPtr&, const HttpRequest&);
  TcpServer server_;
  HttpCallback httpCallback_;
};

}  // namespace net
}  // namespace tmuduo

#endif  // TMUDUO_NET_HTTP_HTTPSERVER_H_