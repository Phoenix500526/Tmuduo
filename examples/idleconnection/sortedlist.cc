#include <stdio.h>
#include <unistd.h>
#include <list>
#include "base/Logging.h"
#include "net/EventLoop.h"
#include "net/TcpServer.h"

using namespace tmuduo;
using namespace tmuduo::net;
using std::string;

class EchoServer : noncopyable {
 public:
  EchoServer(EventLoop* loop, const InetAddress& listenAddr, int idleSeconds)
      : server_(loop, listenAddr, "EchoServer"), idleSeconds_(idleSeconds) {
    server_.setConnectionCallback(
        std::bind(&EchoServer::onConnection, this, _1));
    server_.setMessageCallback(
        std::bind(&EchoServer::onMessage, this, _1, _2, _3));
    loop->runEvery(1.0, std::bind(&EchoServer::onTimer, this));
    dumpConnectionList();
  }
  ~EchoServer() = default;

  void start() { server_.start(); }

 private:
  using WeakTcpConnectionPtr = std::weak_ptr<TcpConnection>;
  using WeakTcpConnectionList = std::list<WeakTcpConnectionPtr>;

  struct Node : copyable {
    Timestamp lastReceiveTime;
    WeakTcpConnectionList::iterator position;
  };

  void onConnection(const TcpConnectionPtr& conn) {
    LOG_INFO << "EchoServer - " << conn->peerAddress().toIpPort() << " -> "
             << conn->localAddress().toIpPort() << " is "
             << (conn->connected() ? "UP" : "DOWN");
    if (conn->connected()) {
      Node node;
      node.lastReceiveTime = Timestamp::now();
      connectionList_.push_back(conn);
      node.position = --connectionList_.end();
      conn->setContext(node);
    } else {
      assert(!conn->getContext().empty());
      const Node& node = boost::any_cast<const Node&>(conn->getContext());
      connectionList_.erase(node.position);
    }
    dumpConnectionList();
  }

  void onMessage(const TcpConnectionPtr& conn, Buffer* buf, Timestamp time) {
    string msg(buf->retrieveAllAsString());
    LOG_INFO << conn->name() << " echo " << msg.size() << " bytes at "
             << time.toString();
    conn->send(msg);
    assert(!conn->getContext().empty());
    Node* node = boost::any_cast<Node>(conn->getMutableContext());
    node->lastReceiveTime = time;
    //将 node 移动到 connectionList_ 尾部
    connectionList_.splice(connectionList_.end(), connectionList_,
                           node->position);
    assert(node->position == --connectionList_.end());
    dumpConnectionList();
  }

  void onTimer() {
    dumpConnectionList();
    Timestamp now = Timestamp::now();
    for (auto it = connectionList_.begin(); it != connectionList_.end();) {
      TcpConnectionPtr conn = it->lock();
      if (conn) {
        Node* node = boost::any_cast<Node>(conn->getMutableContext());
        double age = timeDifference(now, node->lastReceiveTime);
        if (age > idleSeconds_) {
          if (conn->connected()) {
            conn->shutdown();
            LOG_INFO << "shutting down " << conn->name();
            conn->forceCloseWithDelay(3.5);
          }
        } else if (age < 0) {
          LOG_WARN << "Time jump";
          node->lastReceiveTime = now;
        } else {
          break;
        }
        ++it;
      } else {
        LOG_WARN << "Expired";
        it = connectionList_.erase(it);
      }
    }
  }

  void dumpConnectionList() const {
    LOG_INFO << "size = " << connectionList_.size();
    for (auto it = connectionList_.begin(); it != connectionList_.end(); ++it) {
      TcpConnectionPtr conn = it->lock();
      if (conn) {
        printf("conn %p\n", get_pointer(conn));
        const Node& n = boost::any_cast<const Node&>(conn->getContext());
        printf("    time %s\n", n.lastReceiveTime.toString().c_str());
      } else {
        printf("expired\n");
      }
    }
  }
  TcpServer server_;
  int idleSeconds_;
  WeakTcpConnectionList connectionList_;
};

int main(int argc, char* argv[]) {
  EventLoop loop;
  InetAddress listenAddr(2020);
  int idleSeconds = 10;
  if (argc > 1) {
    idleSeconds = atoi(argv[1]);
  }
  LOG_INFO << "pid = " << getpid() << ", idle seconds = " << idleSeconds;
  EchoServer server(&loop, listenAddr, idleSeconds);
  server.start();
  loop.loop();
}