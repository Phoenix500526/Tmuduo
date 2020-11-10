#include "net/TcpServer.h"

#include "base/Logging.h"
#include "base/Thread.h"
#include "net/EventLoop.h"
#include "net/InetAddress.h"

#include <stdio.h>
#include <unistd.h>
#include <atomic>
#include <utility>

using namespace tmuduo;
using namespace tmuduo::net;
using std::string;

class EchoServer : noncopyable {
 public:
  EchoServer(EventLoop* loop, const InetAddress& serverAddr)
      : server_(loop, serverAddr, "EchoServer"),
        oldCounter_(0),
        threadNum_(0),
        startTime_(Timestamp::now()) {
    server_.setConnectionCallback(
        std::bind(&EchoServer::onConnection, this, _1));
    server_.setMessageCallback(
        std::bind(&EchoServer::onMessage, this, _1, _2, _3));
    loop->runEvery(3.0, std::bind(&EchoServer::printThroughput, this));
  }
  void setThreadNum(int threadNum) {
    threadNum_ = threadNum;
    server_.setThreadNum(threadNum);
  }
  void start() {
    LOG_INFO << "starting " << threadNum_ << " threads.";
    server_.start();
  }
  ~EchoServer() = default;

 private:
  void onConnection(const TcpConnectionPtr& conn) {
    LOG_TRACE << conn->peerAddress().toIpPort() << " -> "
              << conn->localAddress().toIpPort() << " is "
              << (conn->connected() ? "UP" : "DOWN");
    conn->setTcpNoDelay(true);
  }

  void onMessage(const TcpConnectionPtr& conn, Buffer* buf, Timestamp) {
    size_t len = buf->readableBytes();
    transferred_.fetch_add(len);
    ++receivedMessages_;
    conn->send(buf);
  }

  void printThroughput() {
    Timestamp endTime = Timestamp::now();
    int64_t newCounter = transferred_.load();
    int64_t bytes = newCounter - oldCounter_;
    int64_t msgs = receivedMessages_.exchange(0);
    double time = timeDifference(endTime, startTime_);
    printf("%4.3f MiB/s %4.3f Ki Msgs/s %6.2f bytes per msg\n",
           static_cast<double>(bytes) / time / 1024 / 1024,
           static_cast<double>(msgs) / time / 1024,
           static_cast<double>(bytes) / static_cast<double>(msgs));
    oldCounter_ = newCounter;
    startTime_ = endTime;
  }

  TcpServer server_;
  std::atomic<int64_t> transferred_;
  std::atomic<int64_t> receivedMessages_;
  int64_t oldCounter_;
  int threadNum_;
  Timestamp startTime_;
};

int main(int argc, char* argv[]) {
  LOG_INFO << "pid = " << getpid() << ", tid = " << CurrentThread::tid();
  EventLoop loop;
  InetAddress serverAddr(2020);
  EchoServer server(&loop, serverAddr);
  if (argc > 1) {
    server.setThreadNum(atoi(argv[1]));
  }
  server.start();
  loop.loop();
}