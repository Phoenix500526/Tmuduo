#include "net/TcpConnection.h"
#include "base/Logging.h"
#include "base/WeakCallback.h"
#include "net/Channel.h"
#include "net/EventLoop.h"
#include "net/InetAddress.h"
#include "net/Socket.h"
#include "net/SocketsOps.h"

#include <errno.h>

using namespace tmuduo;
using namespace tmuduo::net;
using std::string;

void tmuduo::net::defaultConnectionCallback(const TcpConnectionPtr& conn) {
  LOG_TRACE << conn->localAddress().toIpPort() << " -> "
            << conn->peerAddress().toIpPort() << " is "
            << (conn->connected() ? "UP" : "DOWN");
  // do not call conn->forceClose(), because some users want to register message
  // callback only.
}

void tmuduo::net::defaultMessageCallback(const TcpConnectionPtr&, Buffer* buf,
                                         Timestamp) {
  buf->retrieveAll();
}

TcpConnection::TcpConnection(EventLoop* loop, const string& nameArg, int sockfd,
                             const InetAddress& localAddr,
                             const InetAddress& peerAddr)
    : loop_(CHECK_NOTNULL(loop)),
      name_(nameArg),
      state_(StateE::kConnecting),
      reading_(true),
      socket_(new Socket(sockfd)),
      channel_(new Channel(loop, sockfd)),
      localAddr_(localAddr),
      peerAddr_(peerAddr),
      highWaterMark_(64 * 1024 * 1024) {
  channel_->setReadCallback(std::bind(&TcpConnection::handleRead, this, _1));
  channel_->setWriteCallback(std::bind(&TcpConnection::handleWrite, this));
  channel_->setCloseCallback(std::bind(&TcpConnection::handleClose, this));
  channel_->setErrorCallback(std::bind(&TcpConnection::handleError, this));
  LOG_DEBUG << "TcpConnection::ctor[" << name_ << "] at " << this
            << " fd = " << sockfd;
  socket_->setKeepAlive(true);
}

TcpConnection::~TcpConnection() {
  LOG_DEBUG << "TcpConnection::dtor[" << name_ << "] at " << this
            << " fd=" << channel_->fd() << " state=" << stateToString();
  assert(state_ == StateE::kDisconnected);
}

bool TcpConnection::getTcpInfo(struct tcp_info* tcpi) const {
  return socket_->getTcpInfo(tcpi);
}

string TcpConnection::getTcpInfoString() const {
  char buf[1024];
  buf[0] = '\0';
  socket_->getTcpInfoString(buf, sizeof buf);
  return buf;
}

void TcpConnection::send(const void* data, int len) {
  send(StringPiece(static_cast<const char*>(data), len));
}

void TcpConnection::sendInLoop(const void* data, size_t len) {
  loop_->assertInLoopThread();
  ssize_t nwrote = 0;
  size_t remaining = len;
  bool faultError = false;
  if (state_ == StateE::kDisconnected) {
    LOG_WARN << "disconnect, give up writing";
    return;
  }
  // if no thing in output queue, try writing directly
  if (!channel_->isWriting() && outputBuffer_.readableBytes() == 0) {
    nwrote = sockets::write(channel_->fd(), data, len);
    if (nwrote >= 0) {
      remaining = len - nwrote;
      if (remaining == 0 && writeCompleteCallback_) {
        loop_->queueInLoop(
            std::bind(writeCompleteCallback_, shared_from_this()));
      }
    } else {
      nwrote = 0;
      if (errno != EWOULDBLOCK) {
        LOG_SYSERR << "TcpConnection::sendInLoop";
        if (errno == EPIPE ||
            errno == ECONNRESET) {  // FIXME:是否还有其他类型的错误?
          faultError = true;
        }
      }
    }
  }
  assert(remaining <= len);
  if (!faultError && remaining > 0) {
    size_t oldLen = outputBuffer_.readableBytes();
    if (oldLen + remaining >= highWaterMark_ && oldLen < highWaterMark_ &&
        highWaterMarkCallback_) {
      loop_->queueInLoop(std::bind(highWaterMarkCallback_, shared_from_this(),
                                   oldLen + remaining));
    }
    outputBuffer_.append(static_cast<const char*>(data) + nwrote, remaining);
    if (!channel_->isWriting()) {
      channel_->enableWriting();
    }
  }
}

// FIXME efficiency!!!
void TcpConnection::send(Buffer* buf) {
  if (state_ == StateE::kConnected) {
    if (loop_->isInLoopThread()) {
      sendInLoop(buf->peek(), buf->readableBytes());
      buf->retrieveAll();
    } else {
      void (TcpConnection::*fp)(string && message) = &TcpConnection::sendInLoop;
      loop_->runInLoop(
          std::bind(fp, shared_from_this(),
                    // buf->retrieveAllAsString() 是匿名对象,属于 rvalue
                    std::bind(std::move<string&>, buf->retrieveAllAsString())));
    }
  }
}

void TcpConnection::send(Buffer&& message) { send(&message); }

void TcpConnection::send(const StringPiece& message) {
  if (state_ == StateE::kConnected) {
    if (loop_->isInLoopThread()) {
      sendInLoop(message);
    } else {
      //使用函数指针的目的是为了要实现重载函数的绑定
      void (TcpConnection::*fp)(const StringPiece& message) =
          &TcpConnection::sendInLoop;
      loop_->runInLoop(std::bind(fp, shared_from_this(), std::cref(message)));
    }
  }
}

void TcpConnection::send(string&& message) {
  if (state_ == StateE::kConnected) {
    if (loop_->isInLoopThread()) {
      sendInLoop(message);
    } else {
      void (TcpConnection::*fp)(string && message) = &TcpConnection::sendInLoop;
      loop_->runInLoop(
          std::bind(fp, this, std::bind(std::move<string&>, message)));
    }
  }
}

void TcpConnection::sendInLoop(const StringPiece& message) {
  sendInLoop(message.data(), message.size());
}

void TcpConnection::sendInLoop(string&& message) {
  sendInLoop(message.c_str(), message.size());
}

void TcpConnection::handleRead(Timestamp receiveTime) {
  loop_->assertInLoopThread();
  int savedErrno = 0;
  ssize_t n = inputBuffer_.readFd(channel_->fd(), &savedErrno);
  if (n > 0) {
    messageCallback_(shared_from_this(), &inputBuffer_, receiveTime);
  } else if (n == 0) {
    handleClose();
  } else {
    errno = savedErrno;
    LOG_SYSERR << "TcpConnection::handleRead";
    handleError();
  }
}

void TcpConnection::handleWrite() {
  loop_->assertInLoopThread();
  if (channel_->isWriting()) {
    ssize_t n = sockets::write(channel_->fd(), outputBuffer_.peek(),
                               outputBuffer_.readableBytes());
    if (n > 0) {
      outputBuffer_.retrieve(n);
      if (outputBuffer_.readableBytes() == 0) {
        channel_->disableWriting();
        if (writeCompleteCallback_) {
          loop_->queueInLoop(
              std::bind(writeCompleteCallback_, shared_from_this()));
        }
        if (state_ == StateE::kDisconnecting) {
          shutdownInLoop();
        }
      }
    } else {
      LOG_SYSERR << "TcpConnection::handleWrite";
    }
  } else {
    LOG_TRACE << "Connection fd = " << channel_->fd()
              << " is down, no more writing";
  }
}

void TcpConnection::handleClose() {
  loop_->assertInLoopThread();
  LOG_TRACE << "fd = " << channel_->fd() << " state = " << stateToString();
  assert(state_ == StateE::kConnected || state_ == StateE::kDisconnecting);
  // we don't close fd, leave it to dtor, so we can find leaks easily.
  setState(StateE::kDisconnected);
  channel_->disableAll();
  TcpConnectionPtr guardThis(shared_from_this());
  connectionCallback_(guardThis);
  closeCallback_(guardThis);
}

void TcpConnection::handleError() {
  int err = sockets::getSocketError(channel_->fd());
  LOG_ERROR << "TcpConnection::handleError [" << name_
            << "] - SO_ERROR = " << err << " " << strerror_tl(err);
}

const char* TcpConnection::stateToString() const {
  switch (state_.load()) {
    case StateE::kDisconnected:
      return "kDisconnected";
    case StateE::kConnecting:
      return "kConnecting";
    case StateE::kConnected:
      return "kConnected";
    case StateE::kDisconnecting:
      return "kDisconnecting";
    default:
      return "unknown state";
  }
}

void TcpConnection::shutdown() {
  // compare_exchange_strong 的第一个参数是左值引用,因此需要一个 Connected
  // 变量来保存相应的值
  static StateE Connected = StateE::kConnected;
  if (state_.compare_exchange_strong(Connected, StateE::kDisconnecting)) {
    loop_->runInLoop(
        std::bind(&TcpConnection::shutdownInLoop, shared_from_this()));
  }
}

void TcpConnection::shutdownInLoop() {
  loop_->assertInLoopThread();
  if (!channel_->isWriting()) {
    socket_->shutdownWrite();
  }
}

void TcpConnection::forceClose() {
  static StateE Connected = StateE::kConnected;
  if (state_.compare_exchange_strong(Connected, StateE::kDisconnecting) ||
      state_ == StateE::kDisconnecting) {
    loop_->queueInLoop(
        std::bind(&TcpConnection::forceCloseInLoop, shared_from_this()));
  }
}

void TcpConnection::forceCloseWithDelay(double seconds) {
  static StateE Connected = StateE::kConnected;
  if (state_.compare_exchange_strong(Connected, StateE::kDisconnecting) ||
      state_ == StateE::kDisconnecting) {
    loop_->runAfter(
        seconds,
        makeWeakCallback(shared_from_this(),
                         &TcpConnection::forceClose));  // not forceCloseInLoop
                                                        // to avoid race
                                                        // condition
  }
}

void TcpConnection::forceCloseInLoop() {
  loop_->assertInLoopThread();
  if (state_ == StateE::kConnected || state_ == StateE::kDisconnecting) {
    // as if we received 0 byte in handleRead();
    handleClose();
  }
}

void TcpConnection::setTcpNoDelay(bool on) { socket_->setTcpNoDelay(on); }

void TcpConnection::connectEstablished() {
  loop_->assertInLoopThread();
  assert(state_ == StateE::kConnecting);
  setState(StateE::kConnected);
  channel_->tie(shared_from_this());
  channel_->enableReading();
  connectionCallback_(shared_from_this());
}

// connectionDestroyed 和 handleClose 中的代码有重复部分,这是因为在某些情况下
//可以不经 handleClose 而是通过 connectDestroyed 来关闭连接
void TcpConnection::connectDestroyed() {
  loop_->assertInLoopThread();
  if (state_ == StateE::kConnected) {
    setState(StateE::kDisconnected);
    channel_->disableAll();
    connectionCallback_(shared_from_this());
  }
  channel_->remove();
}

void TcpConnection::startRead() {
  loop_->runInLoop(std::bind(&TcpConnection::startReadInLoop, this));
}

void TcpConnection::startReadInLoop() {
  loop_->assertInLoopThread();
  if (!reading_ || !channel_->isReading()) {
    channel_->enableReading();
    reading_ = true;
  }
}

void TcpConnection::stopRead() {
  loop_->runInLoop(std::bind(&TcpConnection::stopReadInLoop, this));
}

void TcpConnection::stopReadInLoop() {
  loop_->assertInLoopThread();
  if (reading_ || channel_->isReading()) {
    channel_->disableReading();
    reading_ = false;
  }
}
