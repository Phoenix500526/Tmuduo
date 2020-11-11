#include "examples/hub/codec.h"

#include "base/Logging.h"
#include "net/EventLoop.h"
#include "net/TcpServer.h"

#include <stdio.h>
#include <map>
#include <set>

using namespace tmuduo;
using namespace tmuduo::net;
using std::string;

namespace pubsub {

using ConnectionSubscription = std::set<string>;

class Topic : public tmuduo::copyable {
 public:
  Topic(const string& topic) : topic_(topic) {}

  void add(const TcpConnectionPtr& conn) {
    audiences_.insert(conn);
    if (lastPubTime_.valid()) {
      conn->send(makeMessage());
    }
  }

  void remove(const TcpConnectionPtr& conn) { audiences_.erase(conn); }

  void publish(const string& content, Timestamp time) {
    content_ = content;
    lastPubTime_ = time;
    string message = makeMessage();
    for (auto it = audiences_.begin(); it != audiences_.end(); ++it) {
      (*it)->send(message);
    }
  }

 private:
  string makeMessage() { return "pub " + topic_ + "\r\n" + content_ + "\r\n"; }
  string topic_;
  string content_;
  Timestamp lastPubTime_;
  std::set<TcpConnectionPtr> audiences_;
};

class PubSubServer : tmuduo::noncopyable {
 public:
  PubSubServer(EventLoop* loop, const InetAddress& serverAddr)
      : loop_(loop), server_(loop, serverAddr, "PubSubServer") {
    server_.setConnectionCallback(
        std::bind(&PubSubServer::onConnection, this, _1));
    server_.setMessageCallback(
        std::bind(&PubSubServer::onMessage, this, _1, _2, _3));
    loop_->runEvery(1.0, std::bind(&PubSubServer::timePublish, this));
  }

  void start() { server_.start(); }

 private:
  void onConnection(const TcpConnectionPtr& conn) {
    if (conn->connected()) {
      conn->setContext(ConnectionSubscription());
    } else {
      const ConnectionSubscription& connSub =
          boost::any_cast<const ConnectionSubscription&>(conn->getContext());
      // subtle: doUnsubscribe will erase *it, so increase before calling.
      for (auto it = connSub.cbegin(); it != connSub.end();) {
        doUnsubscribe(conn, *it++);
      }
    }
  }

  void onMessage(const TcpConnectionPtr& conn, Buffer* buf,
                 Timestamp receiveTime) {
    ParseResult result = ParseResult::kSuccess;
    while (result == ParseResult::kSuccess) {
      string cmd;
      string topic;
      string content;
      result = parseMessage(buf, &cmd, &topic, &content);
      if (result == ParseResult::kSuccess) {
        if (cmd == "pub") {
          doPublish(conn->name(), topic, content, receiveTime);
        } else if (cmd == "sub") {
          LOG_INFO << conn->name() << " subscribes " << topic;
          doSubscribe(conn, topic);
        } else if (cmd == "unsub") {
          doUnsubscribe(conn, topic);
        } else {
          conn->shutdown();
          result = ParseResult::kError;
        }
      } else if (result == ParseResult::kError) {
        conn->shutdown();
      }
    }
  }

  void timePublish() {
    Timestamp now = Timestamp::now();
    doPublish("internal", "utc_time", now.toFormattedString(), now);
  }

  void doPublish(const string& name, const string& topic, const string& content,
                 Timestamp time) {
    getTopic(topic).publish(content, time);
  }

  void doSubscribe(const TcpConnectionPtr& conn, const string& topic) {
    auto connSub =
        boost::any_cast<ConnectionSubscription>(conn->getMutableContext());
    connSub->insert(topic);
    getTopic(topic).add(conn);
  }

  void doUnsubscribe(const TcpConnectionPtr& conn, const string& topic) {
    LOG_INFO << conn->name() << " unsubscribes " << topic;
    getTopic(topic).remove(conn);
    // topic could be the one to be destroyed, so don't use it after erasing.
    auto connSub =
        boost::any_cast<ConnectionSubscription>(conn->getMutableContext());
    connSub->erase(topic);
  }

  Topic& getTopic(const string& topic) {
    auto it = topics_.find(topic);
    if (it == topics_.end()) {
      it = topics_.insert(make_pair(topic, Topic(topic))).first;
    }
    return it->second;
  }

  EventLoop* loop_;
  TcpServer server_;
  std::map<string, Topic> topics_;
};

}  // namespace tmuduo

int main(int argc, char* argv[]) {
  if (argc > 1) {
    uint16_t port = static_cast<uint16_t>(atoi(argv[1]));
    EventLoop loop;
    // if(argc > 2){
    //     int inspectPort = atoi(argv[2]);
    // }
    pubsub::PubSubServer server(&loop, InetAddress(port));
    server.start();
    loop.loop();
  } else {
    printf("Usage: %s pubsub_port [inspect_port]\n", argv[0]);
  }
}