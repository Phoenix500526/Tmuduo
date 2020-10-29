#include "net/InetAddress.h"
#include <gtest/gtest.h>
#include "base/Logging.h"

using namespace std;
using namespace tmuduo;
using namespace tmuduo::net;

TEST(InetAddressUnittest, InetAddressInitTest) {
  InetAddress addr0(1234);
  EXPECT_EQ(addr0.toIp(), string("0.0.0.0"));
  EXPECT_EQ(addr0.toIpPort(), string("0.0.0.0:1234"));
  EXPECT_EQ(addr0.toPort(), 1234);

  InetAddress addr1(4321, true);
  EXPECT_EQ(addr1.toIp(), string("127.0.0.1"));
  EXPECT_EQ(addr1.toIpPort(), string("127.0.0.1:4321"));
  EXPECT_EQ(addr1.toPort(), 4321);

  InetAddress addr2("1.2.3.4", 8888);
  EXPECT_EQ(addr2.toIp(), string("1.2.3.4"));
  EXPECT_EQ(addr2.toIpPort(), string("1.2.3.4:8888"));
  EXPECT_EQ(addr2.toPort(), 8888);

  InetAddress addr3("255.255.255.255", 65535);
  EXPECT_EQ(addr3.toIp(), string("255.255.255.255"));
  EXPECT_EQ(addr3.toIpPort(), string("255.255.255.255:65535"));
  EXPECT_EQ(addr3.toPort(), 65535);
}

TEST(InetAddressUnittest, InetAddressResolveTest) {
  InetAddress addr(80);
  if (InetAddress::resolve("google.com", &addr)) {
    LOG_INFO << "google.com resolved to " << addr.toIpPort();
  } else {
    LOG_ERROR << "Unable to resolve google.com";
  }
}