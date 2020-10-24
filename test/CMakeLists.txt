project(TmuduoTest)

add_executable(CountDownLatch_test CountDownLatch_test.cc)
target_include_directories(CountDownLatch_test PUBLIC ${CMAKE_SOURCE_DIR})
target_link_libraries(CountDownLatch_test Base::lib)


add_executable(Mutex_test Mutex_test.cc)
target_include_directories(Mutex_test PUBLIC ${CMAKE_SOURCE_DIR})
target_link_libraries(Mutex_test Base::lib)


add_executable(Exception_test Exception_test.cc)
target_include_directories(Exception_test PUBLIC ${CMAKE_SOURCE_DIR})
target_link_libraries(Exception_test Base::lib)

add_executable(Thread_test Thread_test.cc)
target_include_directories(Thread_test PUBLIC ${CMAKE_SOURCE_DIR})
target_link_libraries(Thread_test Base::lib)

add_executable(LogStream_Unittest LogStream_Unittest.cc)
target_include_directories(LogStream_Unittest PUBLIC ${CMAKE_SOURCE_DIR})
target_link_libraries(LogStream_Unittest Base::lib ${CONAN_LIBS})
add_test(NAME LogStream_Unittest COMMAND LogStream_Unittest)

