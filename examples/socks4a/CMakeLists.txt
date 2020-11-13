add_executable(TcpRealy tcprelay.cc)
target_include_directories(TcpRealy PUBLIC ${CMAKE_SOURCE_DIR})
target_link_libraries(TcpRealy Net::lib)


add_executable(Socks4a socks4a.cc)
target_include_directories(Socks4a PUBLIC ${CMAKE_SOURCE_DIR})
target_link_libraries(Socks4a Net::lib)

add_executable(Balancer balancer.cc)
target_include_directories(Balancer PUBLIC ${CMAKE_SOURCE_DIR})
target_link_libraries(Balancer Net::lib)