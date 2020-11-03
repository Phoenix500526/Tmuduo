#ifndef TMUDUO_NET_HTTP_HTTPRESPONSE_H_
#define TMUDUO_NET_HTTP_HTTPRESPONSE_H_

#include "base/StringPiece.h"
#include "base/TypeCast.h"
#include "base/copyable.h"

#include <map>

namespace tmuduo {
namespace net {

class Buffer;
class HttpResponse : public copyable {
 public:
  enum class HttpStatusCode {
    kUnknown,
    k200Ok = 200,
    k301MovedPermanently = 301,
    k400BadRequest = 400,
    k404NotFound = 404,
  };

  explicit HttpResponse(bool close)
      : statusCode_(HttpStatusCode::kUnknown), closeConnection_(close) {}
  ~HttpResponse() = default;

  void setStatusCode(HttpStatusCode code) { statusCode_ = code; }

  void setStatusMessage(const std::string& message) {
    statusMessage_ = message;
  }

  void setCloseConnection(bool on) { closeConnection_ = on; }

  bool closeConnection() const { return closeConnection_; }

  void setContentType(const std::string& contentType) {
    addHeader("Content-Type", contentType);
  }

  void addHeader(const StringPiece& key, const StringPiece& value) {
    headers_[key.as_string()] = value.as_string();
  }

  void setBody(const std::string& body) { body_ = body; }

  void appendToBuffer(Buffer* output) const;

 private:
  std::map<std::string, std::string> headers_;
  HttpStatusCode statusCode_;
  std::string statusMessage_;
  // FIXME: add http version
  bool closeConnection_;
  std::string body_;
};

}  // namespace net
}  // namespace tmuduo

#endif  // TMUDUO_NET_HTTP_HTTPRESPONSE_H_