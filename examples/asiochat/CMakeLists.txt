add_executable(asio_chat_server server.cc)
target_include_directories(asio_chat_server PUBLIC ${CMAKE_SOURCE_DIR})
target_link_libraries(asio_chat_server Net::lib)

add_executable(asio_chat_client client.cc)
target_include_directories(asio_chat_client PUBLIC ${CMAKE_SOURCE_DIR})
target_link_libraries(asio_chat_client Net::lib)
