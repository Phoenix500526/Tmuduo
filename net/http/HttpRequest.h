#ifndef TMUDUO_NET_HTTP_HTTPREQUEST_H_
#define TMUDUO_NET_HTTP_HTTPREQUEST_H_

#include "base/Timestamp.h"
#include "base/TypeCast.h"
#include "base/copyable.h"

#include <assert.h>
#include <stdio.h>
#include <map>

namespace tmuduo {
namespace net {

class HttpRequest : public tmuduo::copyable {
 public:
  enum class Method { kInvalid, kGet, kPost, kHead, kPut, kDelete };
  enum class Version { kUnknown, kHttp10, kHttp11 };
  HttpRequest() : method_(Method::kInvalid), version_(Version::kUnknown) {}
  ~HttpRequest() = default;
  void setVersion(Version v) { version_ = v; }

  Version version() const { return version_; }

  bool setMethod(const char* start, const char* end) {
    assert(method_ == Method::kInvalid);
    std::string m(start, end);
    if (m == "GET") {
      method_ = Method::kGet;
    } else if (m == "POST") {
      method_ = Method::kPost;
    } else if (m == "HEAD") {
      method_ = Method::kHead;
    } else if (m == "PUT") {
      method_ = Method::kPut;
    } else if (m == "DELETE") {
      method_ = Method::kDelete;
    } else {
      method_ = Method::kInvalid;
    }
    return method_ != Method::kInvalid;
  }

  Method method() const { return method_; }

  const char* methodString() const {
    const char* result = "UNKNOWN";
    switch (method_) {
      case Method::kGet:
        result = "GET";
        break;
      case Method::kPost:
        result = "POST";
        break;
      case Method::kHead:
        result = "HEAD";
        break;
      case Method::kPut:
        result = "PUT";
        break;
      case Method::kDelete:
        result = "DELETE";
        break;
      default:
        break;
    }
    return result;
  }

  void setPath(const char* start, const char* end) { path_.assign(start, end); }

  const std::string& path() const { return path_; }

  void setQuery(const char* start, const char* end) {
    query_.assign(start, end);
  }

  const std::string& query() const { return query_; }

  void setReceiveTime(Timestamp t) { receiveTime_ = t; }

  Timestamp receiveTime() const { return receiveTime_; }
  // colon 代表冒号
  void addHeader(const char* start, const char* colon, const char* end) {
    std::string field(start, colon);
    ++colon;
    while (colon < end && isspace(*colon)) {
      ++colon;
    }
    std::string value(colon, end);
    while (!value.empty() && isspace(value[value.size() - 1])) {
      value.resize(value.size() - 1);
    }
    headers_[field] = value;
  }

  std::string getHeader(const std::string& field) const {
    std::string result;
    std::map<std::string, std::string>::const_iterator it =
        headers_.find(field);
    if (it != headers_.end()) {
      result = it->second;
    }
    return result;
  }

  const std::map<std::string, std::string>& headers() const { return headers_; }

  void swap(HttpRequest& that) {
    std::swap(method_, that.method_);
    std::swap(version_, that.version_);
    path_.swap(that.path_);
    query_.swap(that.query_);
    receiveTime_.swap(that.receiveTime_);
    headers_.swap(that.headers_);
  }

 private:
  Method method_;
  Version version_;
  std::string path_;
  std::string query_;
  Timestamp receiveTime_;
  std::map<std::string, std::string> headers_;
};

}  // namespace net
}  // namespace tmuduo

#endif  // TMUDUO_NET_HTTP_HTTPREQUEST_H_