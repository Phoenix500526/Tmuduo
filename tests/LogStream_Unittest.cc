#include "base/LogStream.h"

#include <gtest/gtest.h>

using tmuduo::LogStream;
using tmuduo::Fmt;
using namespace std;

class LogStreamTest : public ::testing::Test {
 protected:
  LogStreamTest() : buf_(os.buffer()) {}
  ~LogStreamTest() override {}
  LogStream os;
  const LogStream::Buffer& buf_;
};

TEST_F(LogStreamTest, LogStreamBooleansTest) {
  EXPECT_EQ(buf_.toString(), string(""));
  os << true;
  EXPECT_EQ(buf_.toString(), string("1"));
  os << '\n';
  EXPECT_EQ(buf_.toString(), string("1\n"));
  os << false;
  EXPECT_EQ(buf_.toString(), string("1\n0"));
}

TEST_F(LogStreamTest, LogStreamIntegerTest) {
  EXPECT_EQ(buf_.toString(), string(""));
  os << 1;
  EXPECT_EQ(buf_.toString(), string("1"));
  os << 0;
  EXPECT_EQ(buf_.toString(), string("10"));
  os << -1;
  EXPECT_EQ(buf_.toString(), string("10-1"));
  os.resetBuffer();
  EXPECT_EQ(buf_.toString(), string(""));
  os << 0 << " " << 123 << 'x' << 0x64;
  EXPECT_EQ(buf_.toString(), string("0 123x100"));
}

TEST_F(LogStreamTest, LogStreamIntegerLimitsTest) {
  EXPECT_EQ(buf_.toString(), string(""));
  os << -2147483647;
  EXPECT_EQ(buf_.toString(), string("-2147483647"));
  os << static_cast<int>(-2147483647 - 1);
  EXPECT_EQ(buf_.toString(), string("-2147483647-2147483648"));
  os << ' ';
  os << 2147483647;
  EXPECT_EQ(buf_.toString(), string("-2147483647-2147483648 2147483647"));
  os.resetBuffer();

  os << std::numeric_limits<int16_t>::min();
  EXPECT_EQ(buf_.toString(), string("-32768"));
  os.resetBuffer();

  os << std::numeric_limits<int16_t>::max();
  EXPECT_EQ(buf_.toString(), string("32767"));
  os.resetBuffer();

  os << std::numeric_limits<uint16_t>::min();
  EXPECT_EQ(buf_.toString(), string("0"));
  os.resetBuffer();

  os << std::numeric_limits<uint16_t>::max();
  EXPECT_EQ(buf_.toString(), string("65535"));
  os.resetBuffer();

  os << std::numeric_limits<int32_t>::min();
  EXPECT_EQ(buf_.toString(), string("-2147483648"));
  os.resetBuffer();

  os << std::numeric_limits<int32_t>::max();
  EXPECT_EQ(buf_.toString(), string("2147483647"));
  os.resetBuffer();

  os << std::numeric_limits<uint32_t>::min();
  EXPECT_EQ(buf_.toString(), string("0"));
  os.resetBuffer();

  os << std::numeric_limits<uint32_t>::max();
  EXPECT_EQ(buf_.toString(), string("4294967295"));
  os.resetBuffer();

  os << std::numeric_limits<int64_t>::min();
  EXPECT_EQ(buf_.toString(), string("-9223372036854775808"));
  os.resetBuffer();

  os << std::numeric_limits<int64_t>::max();
  EXPECT_EQ(buf_.toString(), string("9223372036854775807"));
  os.resetBuffer();

  os << std::numeric_limits<uint64_t>::min();
  EXPECT_EQ(buf_.toString(), string("0"));
  os.resetBuffer();

  os << std::numeric_limits<uint64_t>::max();
  EXPECT_EQ(buf_.toString(), string("18446744073709551615"));
  os.resetBuffer();

  int16_t a = 0;
  int32_t b = 0;
  int64_t c = 0;
  os << a;
  os << b;
  os << c;
  EXPECT_EQ(buf_.toString(), string("000"));
}

TEST_F(LogStreamTest, LogStreamFloatsTest) {
  os << 0.0;
  EXPECT_EQ(buf_.toString(), string("0"));
  os.resetBuffer();

  os << 1.0;
  EXPECT_EQ(buf_.toString(), string("1"));
  os.resetBuffer();

  os << 0.1;
  EXPECT_EQ(buf_.toString(), string("0.1"));
  os.resetBuffer();

  os << 0.05;
  EXPECT_EQ(buf_.toString(), string("0.05"));
  os.resetBuffer();

  os << 0.15;
  EXPECT_EQ(buf_.toString(), string("0.15"));
  os.resetBuffer();

  double pi = 3.14156;
  os << pi;
  EXPECT_EQ(buf_.toString(), string("3.14156"));
  os.resetBuffer();

  double a = 0.015;
  os << a;
  EXPECT_EQ(buf_.toString(), string("0.015"));
  os.resetBuffer();

  double b = 0.123;
  os << b + a;
  EXPECT_EQ(buf_.toString(), string("0.138"));
  os.resetBuffer();

  EXPECT_EQ(a + b != pi, true);

  os << 1.23456789;
  EXPECT_EQ(buf_.toString(), string("1.23456789"));
  os.resetBuffer();

  os << 1.23456;
  EXPECT_EQ(buf_.toString(), string("1.23456"));
  os.resetBuffer();

  os << -123.456;
  EXPECT_EQ(buf_.toString(), string("-123.456"));
  os.resetBuffer();
}

TEST_F(LogStreamTest, LogStreamVoidTest) {
  os << static_cast<void*>(0);
  EXPECT_EQ(buf_.toString(), string("0x0"));
  os.resetBuffer();

  os << reinterpret_cast<void*>(8888);
  EXPECT_EQ(buf_.toString(), string("0x22B8"));
  os.resetBuffer();
}

TEST_F(LogStreamTest, LogStreamStringsTest) {
  os << "Hello ";
  EXPECT_EQ(buf_.toString(), string("Hello "));

  string world = "World";
  os << world;
  EXPECT_EQ(buf_.toString(), string("Hello World"));
}

TEST_F(LogStreamTest, LogStreamFmtsTest) {
  os << Fmt("%4d", 1);
  EXPECT_EQ(buf_.toString(), string("   1"));
  os.resetBuffer();

  os << Fmt("%4.2f", 1.2);
  EXPECT_EQ(buf_.toString(), string("1.20"));
  os.resetBuffer();

  os << Fmt("%4.2f", 1.2) << Fmt("%4d", 43);
  EXPECT_EQ(buf_.toString(), string("1.20  43"));
  os.resetBuffer();
}

TEST_F(LogStreamTest, LogStreamLongTest) {
  for (int i = 0; i < 399; ++i) {
    os << "123456789 ";
    EXPECT_EQ(buf_.length(), 10 * (i + 1));
    EXPECT_EQ(buf_.avail(), 4000 - 10 * (i + 1));
  }

  os << "abcdefghi ";
  EXPECT_EQ(buf_.length(), 3990);
  EXPECT_EQ(buf_.avail(), 10);

  os << "abcdefghi";
  EXPECT_EQ(buf_.length(), 3999);
  EXPECT_EQ(buf_.avail(), 1);
}