add_executable(Asio_tutorial_timer2 timer2.cc)
target_include_directories(Asio_tutorial_timer2 PUBLIC ${CMAKE_SOURCE_DIR})
target_link_libraries(Asio_tutorial_timer2 Net::lib)

add_executable(Asio_tutorial_timer3 timer3.cc)
target_include_directories(Asio_tutorial_timer3 PUBLIC ${CMAKE_SOURCE_DIR})
target_link_libraries(Asio_tutorial_timer3 Net::lib)

add_executable(Asio_tutorial_timer4 timer4.cc)
target_include_directories(Asio_tutorial_timer4 PUBLIC ${CMAKE_SOURCE_DIR})
target_link_libraries(Asio_tutorial_timer4 Net::lib)

add_executable(Asio_tutorial_timer5 timer5.cc)
target_include_directories(Asio_tutorial_timer5 PUBLIC ${CMAKE_SOURCE_DIR})
target_link_libraries(Asio_tutorial_timer5 Net::lib)

add_executable(Asio_tutorial_timer6 timer6.cc)
target_include_directories(Asio_tutorial_timer6 PUBLIC ${CMAKE_SOURCE_DIR})
target_link_libraries(Asio_tutorial_timer6 Net::lib)

add_executable(Asio_tutorial_timer7 timer7.cc)
target_include_directories(Asio_tutorial_timer7 PUBLIC ${CMAKE_SOURCE_DIR})
target_link_libraries(Asio_tutorial_timer7 Net::lib)