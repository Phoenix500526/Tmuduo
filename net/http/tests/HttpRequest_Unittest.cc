#include "net/Buffer.h"
#include "net/http/HttpContext.h"

#include <gtest/gtest.h>

#include "base/Logging.h"

using std::string;
using namespace tmuduo;
using namespace tmuduo::net;

TEST(HttpRequestTest, ParseRequestAllInOneTest) {
  HttpContext context;
  Buffer input;
  input.append(
      "GET /index.html HTTP/1.1\r\n"
      "Host: www.google.com\r\n"
      "\r\n");
  EXPECT_EQ(context.parseRequest(&input, Timestamp::now()), true);
  EXPECT_EQ(context.gotAll(), true);
  const HttpRequest& request = context.request();
  EXPECT_EQ(request.method(), HttpRequest::Method::kGet);
  EXPECT_EQ(request.path(), string("/index.html"));
  EXPECT_EQ(request.version(), HttpRequest::Version::kHttp11);
  EXPECT_EQ(request.getHeader("Host"), string("www.google.com"));
  EXPECT_EQ(request.getHeader("User-Agent"), string(""));
}

TEST(HttpRequestTest, ParseRequestInTwoPiecesTest) {
  string all(
      "GET /index.html HTTP/1.1\r\n"
      "Host: www.google.com\r\n"
      "\r\n");

  for (size_t sz1 = 0; sz1 < all.size(); ++sz1) {
    HttpContext context;
    Buffer input;
    input.append(all.c_str(), sz1);
    EXPECT_EQ(context.parseRequest(&input, Timestamp::now()), true);
    EXPECT_EQ(context.gotAll(), false);

    size_t sz2 = all.size() - sz1;
    input.append(all.c_str() + sz1, sz2);
    EXPECT_EQ(context.parseRequest(&input, Timestamp::now()), true);
    EXPECT_EQ(context.gotAll(), true);
    const HttpRequest& request = context.request();
    EXPECT_EQ(request.method(), HttpRequest::Method::kGet);
    EXPECT_EQ(request.path(), string("/index.html"));
    EXPECT_EQ(request.version(), HttpRequest::Version::kHttp11);
    EXPECT_EQ(request.getHeader("Host"), string("www.google.com"));
    EXPECT_EQ(request.getHeader("User-Agent"), string(""));
  }
}

TEST(HttpRequestTest, ParseRequestEmptyHeaderValueTest) {
  HttpContext context;
  Buffer input;
  input.append(
      "GET /index.html HTTP/1.1\r\n"
      "Host: www.google.com\r\n"
      "User-Agent:\r\n"
      "Accept-Encoding: \r\n"
      "\r\n");

  EXPECT_EQ(context.parseRequest(&input, Timestamp::now()), true);
  EXPECT_EQ(context.gotAll(), true);
  const HttpRequest& request = context.request();
  EXPECT_EQ(request.method(), HttpRequest::Method::kGet);
  EXPECT_EQ(request.path(), string("/index.html"));
  EXPECT_EQ(request.version(), HttpRequest::Version::kHttp11);
  EXPECT_EQ(request.getHeader("Host"), string("www.google.com"));
  EXPECT_EQ(request.getHeader("User-Agent"), string(""));
  EXPECT_EQ(request.getHeader("Accept-Encoding"), string(""));
}
