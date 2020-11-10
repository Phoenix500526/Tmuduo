#include "base/Logging.h"
#include "base/Thread.h"
#include "net/EventLoop.h"
#include "net/InetAddress.h"
#include "net/TcpServer.h"

#include <stdio.h>
#include <unistd.h>
#include <atomic>
#include <utility>

using namespace tmuduo;
using namespace tmuduo::net;

class DiscardServer : noncopyable {
 public:
  DiscardServer(EventLoop* loop, const InetAddress& listenAddr)
      : server_(loop, listenAddr, "DiscardServer"),
        oldCounter_(0),
        startTime_(Timestamp::now()),
        threadNum_(0) {
    server_.setConnectionCallback(
        std::bind(&DiscardServer::onConnection, this, _1));
    server_.setMessageCallback(
        std::bind(&DiscardServer::onMessage, this, _1, _2, _3));
    loop->runEvery(3.0, std::bind(&DiscardServer::printThroughput, this));
  }
  ~DiscardServer() = default;
  void start() {
    LOG_INFO << "starting " << threadNum_ << " threads.";
    server_.start();
  }
  void setThreadNum(int threadNum) {
    threadNum_ = threadNum;
    server_.setThreadNum(threadNum);
  }

 private:
  void onConnection(const TcpConnectionPtr& conn) {
    LOG_TRACE << conn->peerAddress().toIpPort() << " -> "
              << conn->localAddress().toIpPort() << " is "
              << (conn->connected() ? "UP" : "DOWN");
  }
  void onMessage(const TcpConnectionPtr& conn, Buffer* buf, Timestamp) {
    size_t len = buf->readableBytes();
    transferred_.fetch_add(len);
    ++receivedMessages_;
    buf->retrieveAll();
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
  Timestamp startTime_;
  int threadNum_;
};

int main(int argc, char* argv[]) {
  LOG_INFO << "pid = " << getpid() << ", tid = " << CurrentThread::tid();
  EventLoop loop;
  InetAddress listenAddr(2020);
  DiscardServer server(&loop, listenAddr);
  if (argc > 1) {
    server.setThreadNum(atoi(argv[1]));
  }
  server.start();
  loop.loop();
}