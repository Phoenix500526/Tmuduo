#include "net/TcpClient.h"
#include "base/Logging.h"
#include "net/Connector.h"
#include "net/EventLoop.h"
#include "net/SocketsOps.h"

#include <stdio.h>

using namespace tmuduo;
using namespace tmuduo::net;

namespace tmuduo {
namespace net {
namespace detail {

void removeConnection(EventLoop* loop, const TcpConnectionPtr& conn) {
  loop->queueInLoop(std::bind(&TcpConnection::connectDestroyed, conn));
}

void removeConnector(const ConnectorPtr& connector) {}

}  // namespace detail
}  // namespace net
}  // namespace tmuduo

TcpClient::TcpClient(EventLoop* loop, const InetAddress& serverAddr,
                     const std::string& nameArg)
    : loop_(CHECK_NOTNULL(loop)),
      connector_(new Connector(loop, serverAddr)),
      name_(nameArg),
      connectionCallback_(defaultConnectionCallback),
      messageCallback_(defaultMessageCallback),
      retry_(false),
      connect_(true),
      nextConnId_(1) {
  connector_->setNewConnectionCallback(
      std::bind(&TcpClient::newConnection, this, _1));
  // FIXME: setConnectFailedCallback
  LOG_INFO << "TcpClient::TcpClient[" << name_ << "] - connector "
           << get_pointer(connector_);
}

TcpClient::~TcpClient() {
  LOG_INFO << "TcpClient::~TcpClient[" << name_ << "] - connector "
           << get_pointer(connector_);
  TcpConnectionPtr conn;
  bool unique = false;
  {
    UniqueLock lock(mutex_);
    //判断当前的 TcpClient 是否独占了对应的 TcpConnection
    unique = connection_.unique();
    conn = connection_;
  }
  if (conn) {
    assert(loop_ == conn->getLoop());
    CloseCallback cb = std::bind(&detail::removeConnection, loop_, _1);
    loop_->runInLoop(std::bind(&TcpConnection::setCloseCallback, conn, cb));
    if (unique) {
      //独占状态下可强制退出
      conn->forceClose();
    }
  } else {
    connector_->stop();
    // FIXME: HACK
    loop_->runAfter(1, std::bind(&detail::removeConnector, connector_));
  }
}

void TcpClient::connect() {
  // FIXME: check state
  LOG_INFO << "TcpClient::connect[" << name_ << "] - connecting to "
           << connector_->serverAddr().toIpPort();
  connect_ = true;
  connector_->start();
}

void TcpClient::disconnect() {
  connect_ = false;
  {
    UniqueLock lock(mutex_);
    if (connection_) {
      connection_->shutdown();
    }
  }
}

void TcpClient::stop() {
  connect_ = false;
  connector_->stop();
}

void TcpClient::newConnection(int sockfd) {
  loop_->assertInLoopThread();
  InetAddress peerAddr(sockets::getPeerAddr(sockfd));
  char buf[32];
  snprintf(buf, sizeof buf, ":%s#%d", peerAddr.toIpPort().c_str(), nextConnId_);
  ++nextConnId_;
  std::string connName = name_ + buf;
  InetAddress localAddr(sockets::getLocalAddr(sockfd));
  // FIXME poll with zero timeout to double confirm the new connection
  TcpConnectionPtr conn = std::make_shared<TcpConnection>(
      loop_, connName, sockfd, localAddr, peerAddr);
  conn->setConnectionCallback(connectionCallback_);
  conn->setMessageCallback(messageCallback_);
  conn->setWriteCompleteCallback(writeCompleteCallback_);
  conn->setCloseCallback(std::bind(&TcpClient::removeConnection, this, _1));
  {
    UniqueLock lock(mutex_);
    connection_ = conn;
  }
  conn->connectEstablished();
}

void TcpClient::removeConnection(const TcpConnectionPtr& conn) {
  loop_->assertInLoopThread();
  assert(loop_ == conn->getLoop());
  {
    UniqueLock lock(mutex_);
    assert(connection_ == conn);
    connection_.reset();
  }
  loop_->queueInLoop(std::bind(&TcpConnection::connectDestroyed, conn));
  if (retry_ && connect_) {
    LOG_INFO << "TcpClient::connect[" << name_ << "] - Reconnecting to "
             << connector_->serverAddr().toIpPort();
    connector_->restart();
  }
}