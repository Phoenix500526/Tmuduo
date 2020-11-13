add_executable(Idleconnection_echo echo.cc main.cc)
target_include_directories(Idleconnection_echo PUBLIC ${CMAKE_SOURCE_DIR})
target_link_libraries(Idleconnection_echo Net::lib)

add_executable(Idleconnection_echo2 sortedlist.cc)
target_include_directories(Idleconnection_echo2 PUBLIC ${CMAKE_SOURCE_DIR})
target_link_libraries(Idleconnection_echo2 Net::lib)