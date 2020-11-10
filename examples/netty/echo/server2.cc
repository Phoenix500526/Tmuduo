#include "net/TcpServer.h"

#include "base/FileUtil.h"
#include "base/Logging.h"
#include "base/ProcessInfo.h"
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
      : server_(loop, serverAddr, "EchoServer"), startTime_(Timestamp::now()) {
    server_.setConnectionCallback(
        std::bind(&EchoServer::onConnection, this, _1));
    server_.setMessageCallback(
        std::bind(&EchoServer::onMessage, this, _1, _2, _3));
    loop->runEvery(5.0, std::bind(&EchoServer::printThroughput, this));
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
    if (conn->connected()) {
      ++connecions_;
    } else {
      --connecions_;
    }
  }

  void onMessage(const TcpConnectionPtr& conn, Buffer* buf, Timestamp) {
    size_t len = buf->readableBytes();
    transferredBytes_.fetch_add(len);
    ++receivedMessages_;
    conn->send(buf);
  }

  void printThroughput() {
    Timestamp endTime = Timestamp::now();
    double bytes = static_cast<double>(transferredBytes_.exchange(0));
    int64_t msgs = receivedMessages_.exchange(0);
    double bytesPerMsg = msgs > 0 ? bytes / msgs : 0;
    double time = timeDifference(endTime, startTime_);
    printf("%.3f MiB/s %.2f Kilo Msgs/s %.2f bytes per msg, ",
           bytes / time / 1024 / 1024, static_cast<double>(msgs) / time / 1000,
           bytesPerMsg);
    printConnection();
    fflush(stdout);
    startTime_ = endTime;
  }

  void printConnection() {
    string procStatus = ProcessInfo::procStatus();
    printf("%d conn, files %d, VmSize %ld KiB, RSS %ld KiB, ",
           connecions_.load(), ProcessInfo::openedFiles(),
           getLong(procStatus, "VmSize:"), getLong(procStatus, "VmRSS:"));
    string meminfo;
    FileUtil::readFile("/proc/meminfo", 65536, &meminfo);
    long total_kb = getLong(meminfo, "MemTotal:");
    long free_kb = getLong(meminfo, "MemFree:");
    long buffers_kb = getLong(meminfo, "Buffers:");
    long cache_kb = getLong(meminfo, "Cached:");
    printf("system memory used %ld KiB\n",
           total_kb - free_kb - buffers_kb - cache_kb);
  }

  long getLong(const string& procStatus, const char* key) {
    long result = 0;
    size_t pos = procStatus.find(key);
    if (pos != string::npos) {
      result = ::atol(procStatus.c_str() + pos + strlen(key));
    }
    return result;
  }

  TcpServer server_;
  std::atomic<int32_t> connecions_;
  std::atomic<int32_t> receivedMessages_;
  std::atomic<int64_t> transferredBytes_;
  int threadNum_;
  Timestamp startTime_;
};

int main(int argc, char* argv[]) {
  LOG_INFO << "pid = " << getpid() << ", tid = " << CurrentThread::tid()
           << ", max files = " << ProcessInfo::maxOpenFiles();
  Logger::setLogLevel(Logger::WARN);
  EventLoop loop;
  InetAddress serverAddr(2020);
  EchoServer server(&loop, serverAddr);
  if (argc > 1) {
    server.setThreadNum(atoi(argv[1]));
  }
  server.start();
  loop.loop();
}