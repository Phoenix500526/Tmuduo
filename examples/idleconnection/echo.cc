#include "examples/idleconnection/echo.h"
#include "base/Logging.h"
#include "net/EventLoop.h"

#include <assert.h>
#include <stdio.h>

using namespace tmuduo;
using namespace tmuduo::net;
using std::string;

EchoServer::EchoServer(EventLoop* loop, const InetAddress& listenAddr,
                       int idleSeconds)
    : server_(loop, listenAddr, "EchoServer"), connectionBuckets_(idleSeconds) {
  server_.setConnectionCallback(std::bind(&EchoServer::onConnection, this, _1));
  server_.setMessageCallback(
      std::bind(&EchoServer::onMessage, this, _1, _2, _3));
  loop->runEvery(1.0, std::bind(&EchoServer::onTimer, this));
  connectionBuckets_.resize(idleSeconds);
  dumpConnectionBuckets();
}

void EchoServer::start() { server_.start(); }

void EchoServer::onConnection(const TcpConnectionPtr& conn) {
  LOG_INFO << "EchoServer - " << conn->peerAddress().toIpPort() << " -> "
           << conn->localAddress().toIpPort() << " is "
           << (conn->connected() ? "UP" : "DOWN");
  if (conn->connected()) {
    EntryPtr entry(new Entry(conn));
    connectionBuckets_.back().insert(entry);
    dumpConnectionBuckets();
    WeakEntryPtr WeakEntry(entry);
    //此处需要保存弱引用，因为在后续收到消息时我们还需要用到它
    //如果保存的是强引用，则当 Entry 被提出 timing wheel 时
    //就不会自动析构，自然也就无法踢掉空闲连接
    conn->setContext(WeakEntry);
  } else {
    assert(!conn->getContext().empty());
    WeakEntryPtr weakEntry(boost::any_cast<WeakEntryPtr>(conn->getContext()));
    LOG_DEBUG << "Entry use_count = " << weakEntry.use_count();
  }
}

void EchoServer::onMessage(const TcpConnectionPtr& conn, Buffer* buf,
                           Timestamp time) {
  string msg(buf->retrieveAllAsString());
  LOG_INFO << conn->name() << " echo " << msg.size() << " bytes at "
           << time.toString();
  conn->send(msg);
  assert(!conn->getContext().empty());
  WeakEntryPtr weakEntry(boost::any_cast<WeakEntryPtr>(conn->getContext()));
  // 为什么要把 Entry 作为 TcpConnection 的 context 保存，如果再
  // 创建一个新的 Entry 会怎样？
  // 假设现在客户端连接到服务器，发送消息后关闭了连接，此时 getContext 返回空
  // 那么server 就会插入一个empty Entry，自然也就不用在更新计数。如果创建了
  // 一个新的 Entry，则需要等待超时以后才能踢掉该链接
  EntryPtr entry(weakEntry.lock());
  if (entry) {
    connectionBuckets_.back().insert(entry);
    dumpConnectionBuckets();
  }
}

void EchoServer::onTimer() {
  connectionBuckets_.push_back(Bucket());
  dumpConnectionBuckets();
}

void EchoServer::dumpConnectionBuckets() const {
  LOG_INFO << "size = " << connectionBuckets_.size();
  int idx = 0;
  for (auto bucketI = connectionBuckets_.begin();
       bucketI != connectionBuckets_.end(); ++bucketI, ++idx) {
    const Bucket& bucket = *bucketI;
    printf("[%d] len = %zd : ", idx, bucket.size());
    for (const auto& it : bucket) {
      bool connectionDead = it->weakConn_.expired();
      printf("%p(%ld)%s, ", get_pointer(it), it.use_count(),
             connectionDead ? "DEAD" : "");
    }
    puts("");
  }
}