#include "examples/hub/pubsub.h"
#include "examples/hub/codec.h"

using namespace tmuduo;
using namespace tmuduo::net;
using namespace pubsub;
using std::string;

PubSubClient::PubSubClient(EventLoop* loop, const InetAddress& hubAddr,
                           const string& name)
    : client_(loop, hubAddr, name) {
  // FIXME: dtor is not thread safe
  client_.setConnectionCallback(
      std::bind(&PubSubClient::onConnection, this, _1));
  client_.setMessageCallback(
      std::bind(&PubSubClient::onMessage, this, _1, _2, _3));
}

void PubSubClient::start() { client_.connect(); }

void PubSubClient::stop() { client_.disconnect(); }

bool PubSubClient::connected() const { return conn_ && conn_->connected(); }

bool PubSubClient::subscribe(const string& topic, const SubscribeCallback& cb) {
  subscribeCallback_ = cb;
  return send("sub " + topic + "\r\n");
}

void PubSubClient::unsubscribe(const string& topic) {
  send("unsub " + topic + "\r\n");
}

bool PubSubClient::publish(const string& topic, const string& content) {
  return send("pub " + topic + "\r\n" + content + "\r\n");
}

void PubSubClient::onConnection(const TcpConnectionPtr& conn) {
  if (conn->connected()) {
    conn_ = conn;
    // FIXME: re-sub
  } else {
    conn_.reset();
  }
  if (connectionCallback_) {
    connectionCallback_(this);
  }
}

void PubSubClient::onMessage(const TcpConnectionPtr& conn, Buffer* buf,
                             Timestamp receiveTime) {
  ParseResult result = ParseResult::kSuccess;
  while (result == ParseResult::kSuccess) {
    string cmd, topic, content;
    result = parseMessage(buf, &cmd, &topic, &content);
    if (result == ParseResult::kSuccess) {
      if (cmd == "pub" && subscribeCallback_) {
        subscribeCallback_(topic, content, receiveTime);
      }
    } else if (result == ParseResult::kError) {
      conn->shutdown();
    }
  }
}

bool PubSubClient::send(string&& message) {
  bool succeed = false;
  if (connected()) {
    conn_->sendByRvalue(std::move(message));
    succeed = true;
  }
  return succeed;
}