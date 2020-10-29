#ifndef TMUDUO_NET_CALLBACKS_H_
#define TMUDUO_NET_CALLBACKS_H_

#include "base/Timestamp.h"

#include <functional>
#include <memory>

namespace tmuduo {

using std::placeholders::_1;
using std::placeholders::_2;
using std::placeholders::_3;

namespace net {

class Buffer;
class TcpConnection;
using TcpConnectionPtr = std::shared_ptr<TcpConnection>;
using TimerCallback = std::function<void()>;
using ConnectionCallback = std::function<void(const TcpConnectionPtr&)>;
using CloseCallback = std::function<void(const TcpConnectionPtr&)>;
using WriteCompleteCallback = std::function<void(const TcpConnectionPtr&)>;
using HighWaterMarkCallback =
    std::function<void(const TcpConnectionPtr&, size_t)>;

// the data has been read to (buf, len)
using MessageCallback =
    std::function<void(const TcpConnectionPtr&, Buffer*, Timestamp)>;

void defaultConnectionCallback(const TcpConnectionPtr& conn);
void defaultMessageCallback(const TcpConnectionPtr& conn, Buffer* buffer,
                            Timestamp receiveTime);

}  // namespace net
}  // namespace tmuduo

#endif  // TMUDUO_NET_CALLBACKS_H_