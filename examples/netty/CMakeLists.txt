add_executable(Netty_discard_server discard/server.cc)
target_include_directories(Netty_discard_server PUBLIC ${CMAKE_SOURCE_DIR})
target_link_libraries(Netty_discard_server Net::lib)

add_executable(Netty_discard_client discard/client.cc)
target_include_directories(Netty_discard_client PUBLIC ${CMAKE_SOURCE_DIR})
target_link_libraries(Netty_discard_client Net::lib)