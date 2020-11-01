project(TmuduoNet)

set(NET_SRC 
    SocketsOps.cc
    Socket.cc
    Acceptor.cc
    InetAddress.cc
    Buffer.cc
    Timer.cc
    Channel.cc
    Poller.cc
    poller/DefaultPoller.cc
    poller/EPollPoller.cc
    poller/PollPoller.cc
    TimerQueue.cc
    EventLoop.cc
    EventLoopThread.cc
    EventLoopThreadPool.cc
    TcpConnection.cc)

add_library(${PROJECT_NAME} STATIC ${NET_SRC})
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_SOURCE_DIR})
target_link_libraries(${PROJECT_NAME} PRIVATE Threads::Threads Base::lib)
add_library(Net::lib ALIAS ${PROJECT_NAME})