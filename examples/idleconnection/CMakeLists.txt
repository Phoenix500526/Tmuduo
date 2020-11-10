add_executable(idleconnection_echo echo.cc main.cc)
target_include_directories(idleconnection_echo PUBLIC ${CMAKE_SOURCE_DIR})
target_link_libraries(idleconnection_echo Net::lib)

add_executable(idleconnection_echo2 sortedlist.cc)
target_include_directories(idleconnection_echo2 PUBLIC ${CMAKE_SOURCE_DIR})
target_link_libraries(idleconnection_echo2 Net::lib)