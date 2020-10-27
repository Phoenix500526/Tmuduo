project(TmuduoNet)

set(NET_SRC 
    SocketsOps.cc
    InetAddress.cc)

add_library(${PROJECT_NAME} STATIC ${NET_SRC})
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_SOURCE_DIR})
target_link_libraries(${PROJECT_NAME} PRIVATE Threads::Threads)

add_library(Net::lib ALIAS ${PROJECT_NAME})