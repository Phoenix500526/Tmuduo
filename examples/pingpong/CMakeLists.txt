add_executable(PingPong_server server.cc)
target_include_directories(PingPong_server PUBLIC ${CMAKE_SOURCE_DIR})
target_link_libraries(PingPong_server Net::lib)

add_executable(PingPong_client client.cc)
target_include_directories(PingPong_client PUBLIC ${CMAKE_SOURCE_DIR})
target_link_libraries(PingPong_client Net::lib)

add_executable(PingPong_bench bench.cc)
target_include_directories(PingPong_bench PUBLIC ${CMAKE_SOURCE_DIR})
target_link_libraries(PingPong_bench  Net::lib)