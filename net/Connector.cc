#include "net/Connector.h"
#include "base/Logging.h"
#include "net/Channel.h"
#include "net/EventLoop.h"
#include "net/SocketsOps.h"

#include <errno.h>
using namespace tmuduo;
using namespace tmuduo::net;

const int Connector::kMaxRetryDelayMs;

Connector::Connector(EventLoop* loop, const InetAddress& serverAddr)
    : loop_(loop),
      timerId(),
      serverAddr_(serverAddr),
      connect_(false),
      state_(States::kDisconnected),
      retryDelayMs_(kInitRetryDelayMs) {
  LOG_DEBUG << "ctor[" << this << "] ";
}

Connector::~Connector() {
  LOG_DEBUG << "dtor[" << this << "]";
  if (!timerId.invalid()) {
    loop_->cancel(timerId);
  }
  assert(!channel_);
}

void Connector::start() {
  connect_ = true;
  loop_->runInLoop(std::bind(&Connector::startInLoop, shared_from_this()));
}

void Connector::startInLoop() {
  loop_->assertInLoopThread();
  assert(state_ == States::kDisconnected);
  if (connect_) {
    connect();
  } else {
    LOG_DEBUG << "do not connect";
  }
}

void Connector::stop() {
  connect_ = false;
  loop_->queueInLoop(std::bind(&Connector::stopInLoop, shared_from_this()));
  if(!timerId.invalid())
    loop_->cancel(timerId);
}

void Connector::stopInLoop() {
  loop_->assertInLoopThread();
  if (state_ == States::kConnecting) {
    setState(States::kDisconnected);
    int sockfd = removeAndResetChannel();
    retry(sockfd);
  }
}

void Connector::connect() {
  int sockfd = sockets::createNonblockingOrDie(serverAddr_.family());
  int ret = sockets::connect(sockfd, serverAddr_.getSockAddr());
  int savedErrno = (ret == 0) ? 0 : errno;
  switch (savedErrno) {
    case 0:
    case EINPROGRESS:
    case EINTR:
    case EISCONN:
      connecting(sockfd);
      break;

    case EAGAIN:
    case EADDRINUSE:
    case EADDRNOTAVAIL:
    case ECONNREFUSED:
    case ENETUNREACH:
      retry(sockfd);
      break;

    case EACCES:
    case EPERM:
    case EAFNOSUPPORT:
    case EALREADY:
    case EBADF:
    case EFAULT:
    case ENOTSOCK:
      LOG_SYSERR << "connect error in Connector::startInLoop " << savedErrno;
      sockets::close(sockfd);
      break;

    default:
      LOG_SYSERR << "Unexpected error in Connector::startInLoop " << savedErrno;
      sockets::close(sockfd);
      // connectErrorCallback_();
      break;
  }
}

void Connector::restart() {
  loop_->assertInLoopThread();
  setState(States::kDisconnected);
  retryDelayMs_ = kInitRetryDelayMs;
  connect_ = true;
  startInLoop();
}

void Connector::connecting(int sockfd) {
  setState(States::kConnecting);
  assert(!channel_);
  channel_.reset(new Channel(loop_, sockfd));
  channel_->setWriteCallback(
      std::bind(&Connector::handleWrite, shared_from_this()));
  channel_->setErrorCallback(
      std::bind(&Connector::handleError, shared_from_this()));
  channel_->enableWriting();
}

// removeAndResetChannel 只在 handleWrite 中被调用
int Connector::removeAndResetChannel() {
  channel_->disableAll();
  channel_->remove();
  int sockfd = channel_->fd();
  // Can't reset channel_ here, because we are inside Channel::handleEvent
  loop_->queueInLoop(std::bind(&Connector::resetChannel, this));  // FIXME:unsafe
  return sockfd;
}

void Connector::resetChannel() { channel_.reset(); }

const char* Connector::stateToString(States s) const {
  switch (state_.load()) {
    case States::kDisconnected:
      return "kDisconnected";
    case States::kConnecting:
      return "kConnecting";
    case States::kConnected:
      return "kConnected";
    default:
      return "unknown state";
  }
}

void Connector::handleWrite() {
  LOG_TRACE << "Connector::handleWrite " << stateToString(state_);
  if (state_ == States::kConnecting) {
    int sockfd = removeAndResetChannel();
    int err = sockets::getSocketError(sockfd);
    if (err) {
      LOG_WARN << "Connector::handleWrite - SO_ERROR = " << err << " "
               << strerror_tl(err);
      retry(sockfd);
    } else if (sockets::isSelfConnect(sockfd)) {
      //即使没有产生错误,也可能会产生自连接
      LOG_WARN << "Connector::handleWrite - Self connect";
      retry(sockfd);
    } else {
      setState(States::kConnected);
      if (connect_) {
        newConnectionCallback_(sockfd);
      } else {
        sockets::close(sockfd);
      }
    }
  } else {
    assert(state_ == States::kDisconnected);
  }
}

void Connector::handleError() {
  LOG_ERROR << "Connector::handleError state = " << stateToString(state_);
  if (state_ == States::kConnecting) {
    int sockfd = removeAndResetChannel();
    int err = sockets::getSocketError(sockfd);
    LOG_TRACE << "SO_ERROR = " << err << " " << strerror_tl(err);
    retry(sockfd);
  }
}

void Connector::retry(int sockfd) {
  sockets::close(sockfd);
  setState(States::kDisconnected);
  if (connect_) {
    LOG_INFO << "Connector::retry - Retry connecting to "
             << serverAddr_.toIpPort() << " in " << retryDelayMs_
             << " milliseconds. ";
    timerId =
        loop_->runAfter(retryDelayMs_ / 1000.0,
                        std::bind(&Connector::startInLoop, shared_from_this()));
    // 退避算法 back-off
    // 重试间隔会逐步延长,但是也会带来生命周期的管理问题.例如在定时器到期之前
    // Connector 对象就已经被析构了,怎么办?
    // 解决方法是在析构函数中 cancel 掉未超时的 timer
    retryDelayMs_ = std::min(retryDelayMs_ * 2, kMaxRetryDelayMs);
  } else {
    LOG_DEBUG << "do not connect";
  }
}
