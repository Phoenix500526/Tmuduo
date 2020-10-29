#include "net/Buffer.h"
#include <gtest/gtest.h>

using std::string;
using tmuduo::net::Buffer;

class BufferUnittest : public ::testing::Test {
 protected:
  Buffer buf_;
};

TEST_F(BufferUnittest, BufferAppendRetrieveTest) {
  EXPECT_EQ(buf_.readableBytes(), 0);
  EXPECT_EQ(buf_.writableBytes(), Buffer::kInitialSize);
  EXPECT_EQ(buf_.prependableBytes(), Buffer::kCheapPrepend);

  const string str1(200, 'x');
  buf_.append(str1);
  EXPECT_EQ(buf_.readableBytes(), str1.size());
  EXPECT_EQ(buf_.writableBytes(), Buffer::kInitialSize - str1.size());
  EXPECT_EQ(buf_.prependableBytes(), Buffer::kCheapPrepend);

  const string str2 = buf_.retrieveAsString(50);
  EXPECT_EQ(str2.size(), 50);
  EXPECT_EQ(buf_.readableBytes(), str1.size() - str2.size());
  EXPECT_EQ(buf_.writableBytes(), Buffer::kInitialSize - str1.size());
  EXPECT_EQ(buf_.prependableBytes(), Buffer::kCheapPrepend + str2.size());
  EXPECT_EQ(str2, string(50, 'x'));

  buf_.append(str1);
  EXPECT_EQ(buf_.readableBytes(), 2 * str1.size() - str2.size());
  EXPECT_EQ(buf_.writableBytes(), Buffer::kInitialSize - 2 * str1.size());
  EXPECT_EQ(buf_.prependableBytes(), Buffer::kCheapPrepend + str2.size());

  const string str3 = buf_.retrieveAllAsString();
  EXPECT_EQ(str3.size(), 350);
  EXPECT_EQ(str3, string(350, 'x'));
  EXPECT_EQ(buf_.readableBytes(), 0);
  EXPECT_EQ(buf_.writableBytes(), Buffer::kInitialSize);
  EXPECT_EQ(buf_.prependableBytes(), Buffer::kCheapPrepend);
}

TEST_F(BufferUnittest, BufferGrowTest) {
  buf_.append(string(400, 'y'));
  EXPECT_EQ(buf_.readableBytes(), 400);
  EXPECT_EQ(buf_.prependableBytes(), Buffer::kCheapPrepend);
  EXPECT_EQ(buf_.writableBytes(), Buffer::kInitialSize - 400);

  buf_.retrieve(50);
  EXPECT_EQ(buf_.readableBytes(), 350);
  EXPECT_EQ(buf_.writableBytes(), Buffer::kInitialSize - 400);
  EXPECT_EQ(buf_.prependableBytes(), Buffer::kCheapPrepend + 50);

  buf_.append(string(1000, 'z'));
  EXPECT_EQ(buf_.readableBytes(), 1350);
  EXPECT_EQ(buf_.writableBytes(), 0);
  EXPECT_EQ(buf_.prependableBytes(), Buffer::kCheapPrepend + 50);

  buf_.retrieveAll();
  EXPECT_EQ(buf_.readableBytes(), 0);
  EXPECT_EQ(buf_.writableBytes(), 1400);
  EXPECT_EQ(buf_.prependableBytes(), Buffer::kCheapPrepend);
}

TEST_F(BufferUnittest, BufferInsideGrowTest) {
  buf_.append(string(800, 'y'));
  EXPECT_EQ(buf_.readableBytes(), 800);
  EXPECT_EQ(buf_.writableBytes(), Buffer::kInitialSize - 800);

  buf_.retrieve(500);
  EXPECT_EQ(buf_.readableBytes(), 300);
  EXPECT_EQ(buf_.writableBytes(), Buffer::kInitialSize - 800);
  EXPECT_EQ(buf_.prependableBytes(), Buffer::kCheapPrepend + 500);

  buf_.append(string(300, 'z'));
  EXPECT_EQ(buf_.readableBytes(), 600);
  EXPECT_EQ(buf_.writableBytes(), Buffer::kInitialSize - 600);
  EXPECT_EQ(buf_.prependableBytes(), Buffer::kCheapPrepend);
}

TEST_F(BufferUnittest, BufferShrinkTest) {
  buf_.append(string(2000, 'y'));
  EXPECT_EQ(buf_.readableBytes(), 2000);
  EXPECT_EQ(buf_.writableBytes(), 0);
  EXPECT_EQ(buf_.prependableBytes(), Buffer::kCheapPrepend);

  buf_.retrieve(1500);
  EXPECT_EQ(buf_.readableBytes(), 500);
  EXPECT_EQ(buf_.writableBytes(), 0);
  EXPECT_EQ(buf_.prependableBytes(), Buffer::kCheapPrepend + 1500);

  buf_.shrink(0);
  EXPECT_EQ(buf_.readableBytes(), 500);
  EXPECT_EQ(buf_.writableBytes(), Buffer::kInitialSize - 500);
  EXPECT_EQ(buf_.retrieveAllAsString(), string(500, 'y'));
  EXPECT_EQ(buf_.prependableBytes(), Buffer::kCheapPrepend);
}

TEST_F(BufferUnittest, BufferPrependTest) {
  buf_.append(string(200, 'y'));
  EXPECT_EQ(buf_.readableBytes(), 200);
  EXPECT_EQ(buf_.writableBytes(), Buffer::kInitialSize - 200);
  EXPECT_EQ(buf_.prependableBytes(), Buffer::kCheapPrepend);

  int x = 0;
  buf_.prepend(&x, sizeof x);
  EXPECT_EQ(buf_.readableBytes(), 204);
  EXPECT_EQ(buf_.writableBytes(), Buffer::kInitialSize - 200);
  EXPECT_EQ(buf_.prependableBytes(), Buffer::kCheapPrepend - 4);
}

TEST_F(BufferUnittest, BufferReadIntTest) {
  buf_.append("HTTP");
  EXPECT_EQ(buf_.readableBytes(), 4);
  EXPECT_EQ(buf_.peekInt8(), 'H');

  EXPECT_EQ(buf_.peekInt16(), (1 << 8) * 'H' + 'T');
  EXPECT_EQ(buf_.peekInt32(),
            (1 << 24) * 'H' + (1 << 16) * 'T' + (1 << 8) * 'T' + 'P');
  EXPECT_EQ(buf_.readInt8(), 'H');
  EXPECT_EQ(buf_.readInt16(), (1 << 8) * 'T' + 'T');
  EXPECT_EQ(buf_.readInt8(), 'P');
  EXPECT_EQ(buf_.readableBytes(), 0);
  EXPECT_EQ(buf_.writableBytes(), Buffer::kInitialSize);

  buf_.appendInt8(-1);
  buf_.appendInt16(-2);
  buf_.appendInt32(-3);
  EXPECT_EQ(buf_.readableBytes(), 7);
  EXPECT_EQ(buf_.readInt8(), -1);
  EXPECT_EQ(buf_.readInt16(), -2);
  EXPECT_EQ(buf_.readInt32(), -3);
}

TEST_F(BufferUnittest, BufferFindEOLTest) {
  buf_.append(string(100000, 'x'));
  EXPECT_EQ(buf_.findEOL(), nullptr);
  EXPECT_EQ(buf_.findEOL(buf_.peek() + 90000), nullptr);
}

void output(Buffer&& buf, const void* inner) {
  Buffer newbuf(std::move(buf));
  EXPECT_EQ(inner, newbuf.peek());
}

TEST_F(BufferUnittest, BufferMoveTest) {
  buf_.append("tmuduo", 6);
  const void* inner = buf_.peek();
  output(std::move(buf_), inner);
}