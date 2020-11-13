add_executable(Multiplex_demux demux.cc)
target_include_directories(Multiplex_demux PUBLIC ${CMAKE_SOURCE_DIR})
target_link_libraries(Multiplex_demux Net::lib)

add_executable(Multiplex_server_simple multiplexer_simple.cc)
target_include_directories(Multiplex_server_simple PUBLIC ${CMAKE_SOURCE_DIR})
target_link_libraries(Multiplex_server_simple Net::lib)

add_executable(Multiplex_server multiplexer.cc)
target_include_directories(Multiplex_server PUBLIC ${CMAKE_SOURCE_DIR})
target_link_libraries(Multiplex_server Net::lib)