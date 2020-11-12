#ifndef TMUDUO_NET_HTTP_HTTPCONTEXT_H_
#define TMUDUO_NET_HTTP_HTTPCONTEXT_H_

#include "base/copyable.h"
#include "net/http/HttpRequest.h"

namespace tmuduo {
namespace net {

class Buffer;
class HttpContext : public copyable {
 public:
  // REST 方法以 HTTP 动词描述行为，因此是没有 Body
  enum class HttpRequestParseState {
    kExpectRequestLine,
    kExpectHeaders,
    kGotAll,
  };

  HttpContext() : state_(HttpRequestParseState::kExpectRequestLine) {}

  bool parseRequest(Buffer* buf, Timestamp receiveTime);
  bool gotAll() const { return state_ == HttpRequestParseState::kGotAll; }

  void reset() {
    state_ = HttpRequestParseState::kExpectRequestLine;
    HttpRequest dummy;
    request_.swap(dummy);
  }

  const HttpRequest& request() const { return request_; }

  HttpRequest& request() { return request_; }

 private:
  bool processRequestLine(const char* begin, const char* end);
  HttpRequestParseState state_;
  HttpRequest request_;
};

}  // namespace net
}  // namespace tmuduo

#endif  // TMUDUO_NET_HTTP_HTTPCONTEXT_H_