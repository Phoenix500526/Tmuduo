#ifndef TMUDUO_EXAMPLES_HUB_PUBSUB_H_
#define TMUDUO_EXAMPLES_HUB_PUBSUB_H_

#include "net/TcpClient.h"

namespace pubsub {

class PubSubClient : tmuduo::noncopyable {
 public:
  using ConnectionCallback = std::function<void(PubSubClient*)>;
  using SubscribeCallback = std::function<void(
      const std::string& topic, const std::string& content, tmuduo::Timestamp)>;
  PubSubClient(tmuduo::net::EventLoop* loop,
               const tmuduo::net::InetAddress& hubAddr,
               const std::string& name);
  // dtor is not thread safe
  ~PubSubClient() = default;
  void start();
  void stop();
  bool connected() const;
  void setConnectionCallback(const ConnectionCallback& cb) {
    connectionCallback_ = cb;
  }
  bool subscribe(const std::string& topic, const SubscribeCallback& cb);
  void unsubscribe(const std::string& topic);
  bool publish(const std::string& topic, const std::string& content);

 private:
  void onConnection(const tmuduo::net::TcpConnectionPtr& conn);
  void onMessage(const tmuduo::net::TcpConnectionPtr& conn,
                 tmuduo::net::Buffer* buf, tmuduo::Timestamp receiveTime);
  bool send(std::string&& message);
  tmuduo::net::TcpClient client_;
  tmuduo::net::TcpConnectionPtr conn_;
  ConnectionCallback connectionCallback_;
  SubscribeCallback subscribeCallback_;
};

}  // namespace pubsub

#endif  // TMUDUO_EXAMPLES_HUB_PUBSUB_H_