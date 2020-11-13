#ifndef TMUDUO_EXAMPLES_SOCKS4A_TUNNEL_H_
#define TMUDUO_EXAMPLES_SOCKS4A_TUNNEL_H_

#include "base/Logging.h"
#include "net/EventLoop.h"
#include "net/InetAddress.h"
#include "net/TcpClient.h"
#include "net/TcpServer.h"

class Tunnel : public std::enable_shared_from_this<Tunnel>,
               public tmuduo::noncopyable {
 public:
  Tunnel(tmuduo::net::EventLoop* loop,
         const tmuduo::net::InetAddress& serverAddr,
         const tmuduo::net::TcpConnectionPtr& serverConn)
      : client_(loop, serverAddr, serverConn->name()), serverConn_(serverConn) {
    LOG_INFO << "Tunnel " << serverConn->peerAddress().toIpPort() << " <-> "
             << serverAddr.toIpPort();
  }
  ~Tunnel() { LOG_INFO << "~Tunnel"; }

  void setup() {
    using std::placeholders::_1;
    using std::placeholders::_2;
    using std::placeholders::_3;
    client_.setConnectionCallback(
        std::bind(&Tunnel::onClientConnection, shared_from_this(), _1));
    client_.setMessageCallback(
        std::bind(&Tunnel::onClientMessage, shared_from_this(), _1, _2, _3));
    serverConn_->setHighWaterMarkCallback(
        std::bind(&Tunnel::onHighWaterMarkWeak,
                  std::weak_ptr<Tunnel>(shared_from_this()),
                  ServerClient::kServer, _1, _2),
        1024 * 1024);
  }

  void connect() { client_.connect(); }

  void disconnect() { client_.disconnect(); }

 private:
  enum class ServerClient { kServer, kClient };
  void onClientConnection(const tmuduo::net::TcpConnectionPtr& conn) {
    using std::placeholders::_1;
    using std::placeholders::_2;
    LOG_DEBUG << (conn->connected() ? "UP" : "DOWN");
    if (conn->connected()) {
      conn->setTcpNoDelay(true);
      conn->setHighWaterMarkCallback(
          std::bind(&Tunnel::onHighWaterMarkWeak,
                    std::weak_ptr<Tunnel>(shared_from_this()),
                    ServerClient::kClient, _1, _2),
          1024 * 1024);
      serverConn_->setContext(conn);
      serverConn_->startRead();
      clientConn_ = conn;
      if (serverConn_->inputBuffer()->readableBytes() > 0) {
        conn->send(serverConn_->inputBuffer());
      }
    } else {
      teardown();
    }
  }

  void onClientMessage(const tmuduo::net::TcpConnectionPtr& conn,
                       tmuduo::net::Buffer* buf, tmuduo::Timestamp) {
    LOG_DEBUG << conn->name() << " " << buf->readableBytes();
    if (serverConn_) {
      serverConn_->send(buf);
    } else {
      buf->retrieveAll();
      abort();
    }
  }

  void onHighWaterMark(ServerClient which,
                       const tmuduo::net::TcpConnectionPtr& conn,
                       size_t bytesToSend) {
    using std::placeholders::_1;
    LOG_INFO << (which == ServerClient::kServer ? "server" : "client")
             << " onHighWaterMark " << conn->name() << " bytes " << bytesToSend;
    if (which == ServerClient::kServer) {
      if (serverConn_->outputBuffer()->readableBytes() > 0) {
        clientConn_->stopRead();
        serverConn_->setWriteCompleteCallback(
            std::bind(&Tunnel::onWriteCompleteWeak,
                      std::weak_ptr<Tunnel>(shared_from_this()),
                      ServerClient::kServer, _1));
      }
    } else {
      if (clientConn_->outputBuffer()->readableBytes() > 0) {
        serverConn_->stopRead();
        clientConn_->setWriteCompleteCallback(
            std::bind(&Tunnel::onWriteCompleteWeak,
                      std::weak_ptr<Tunnel>(shared_from_this()),
                      ServerClient::kClient, _1));
      }
    }
  }

  static void onHighWaterMarkWeak(const std::weak_ptr<Tunnel>& wkTunnel,
                                  ServerClient which,
                                  const tmuduo::net::TcpConnectionPtr& conn,
                                  size_t bytesToSend) {
    std::shared_ptr<Tunnel> tunnel = wkTunnel.lock();
    if (tunnel) {
      tunnel->onHighWaterMark(which, conn, bytesToSend);
    }
  }

  void onWriteComplete(ServerClient which,
                       const tmuduo::net::TcpConnectionPtr& conn) {
    LOG_INFO << (which == ServerClient::kServer ? "server" : "client")
             << " onWriteComplete " << conn->name();
    if (which == ServerClient::kServer) {
      clientConn_->startRead();
      serverConn_->setWriteCompleteCallback(
          tmuduo::net::WriteCompleteCallback());
    } else {
      serverConn_->startRead();
      clientConn_->setWriteCompleteCallback(
          tmuduo::net::WriteCompleteCallback());
    }
  }

  static void onWriteCompleteWeak(const std::weak_ptr<Tunnel>& wkTunnel,
                                  ServerClient which,
                                  const tmuduo::net::TcpConnectionPtr& conn) {
    std::shared_ptr<Tunnel> tunnel = wkTunnel.lock();
    if (tunnel) {
      tunnel->onWriteComplete(which, conn);
    }
  }

  void teardown() {
    client_.setConnectionCallback(tmuduo::net::defaultConnectionCallback);
    client_.setMessageCallback(tmuduo::net::defaultMessageCallback);
    if (serverConn_) {
      serverConn_->setContext(boost::any());
      serverConn_->shutdown();
    }
    clientConn_.reset();
  }

  tmuduo::net::TcpClient client_;
  tmuduo::net::TcpConnectionPtr serverConn_;
  tmuduo::net::TcpConnectionPtr clientConn_;
};

using TunnelPtr = std::shared_ptr<Tunnel>;

#endif  // TMUDUO_EXAMPLES_SOCKS4A_TUNNEL_H_