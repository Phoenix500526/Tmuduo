#include "examples/socks4a/tunnel.h"

#include <stdio.h>

using namespace tmuduo;
using namespace tmuduo::net;
using std::string;

std::vector<InetAddress> g_backends;
thread_local std::map<string, TunnelPtr> t_tunnels;
Mutex g_mutex;
size_t g_current = 0;

void onServerConnection(const TcpConnectionPtr& conn) {
  LOG_DEBUG << (conn->connected() ? "UP" : "DOWN");
  if (conn->connected()) {
    conn->setTcpNoDelay(true);
    conn->stopRead();
    size_t current = 0;
    {
      UniqueLock lock(g_mutex);
      current = g_current;
      g_current = (g_current + 1) % g_backends.size();
    }

    InetAddress backend = g_backends[current];
    TunnelPtr tunnel(new Tunnel(conn->getLoop(), backend, conn));
    tunnel->setup();
    tunnel->connect();
    t_tunnels[conn->name()] = tunnel;
  } else {
    assert(t_tunnels.find(conn->name()) != t_tunnels.end());
    t_tunnels[conn->name()]->disconnect();
    t_tunnels.erase(conn->name());
  }
}

void onServerMessage(const TcpConnectionPtr& conn, Buffer* buf, Timestamp) {
  if (!conn->getContext().empty()) {
    const auto& clientConn =
        boost::any_cast<const TcpConnectionPtr&>(conn->getContext());
    clientConn->send(buf);
  }
}

int main(int argc, char* argv[]) {
  if (argc < 3) {
    fprintf(stderr, "Usage: %s listen_port backend_ip:port [backend_ip:prot]\n",
            argv[0]);
  } else {
    for (int i = 2; i < argc; ++i) {
      string hostport = argv[i];
      size_t colon = hostport.find(':');
      if (colon != string::npos) {
        string ip = hostport.substr(0, colon);
        uint16_t port =
            static_cast<uint16_t>(atoi(hostport.c_str() + colon + 1));
        g_backends.push_back(InetAddress(ip, port));
      } else {
        fprintf(stderr, "invalid backend address %s\n", argv[i]);
        return 1;
      }
    }

    uint16_t port = static_cast<uint16_t>(atoi(argv[1]));
    InetAddress listenAddr(port);

    EventLoop loop;
    TcpServer server(&loop, listenAddr, "TcpBalancer");
    server.setConnectionCallback(onServerConnection);
    server.setMessageCallback(onServerMessage);
    server.setThreadNum(4);
    server.start();
    loop.loop();
  }
}