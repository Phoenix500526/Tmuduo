add_executable(Shorturl shorturl.cc)
target_include_directories(Shorturl PUBLIC ${CMAKE_SOURCE_DIR})
target_link_libraries(Shorturl Net::lib HTTP::lib)