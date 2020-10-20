project(TmuduoTest)

add_executable(UnitTest unit_tests.cc ${CMAKE_SOURCE_DIR}/base/Palindrome.cc)
target_include_directories(UnitTest PUBLIC ${CMAKE_SOURCE_DIR})

target_link_libraries(UnitTest ${CONAN_LIBS})
add_test(NAME UnitTest COMMAND UnitTest)


add_executable(CountDownLatch_test CountDownLatch_test.cc)
target_include_directories(CountDownLatch_test PUBLIC ${CMAKE_SOURCE_DIR})
target_link_libraries(CountDownLatch_test Base::lib)


add_executable(Mutex_test Mutex_test.cc)
target_include_directories(Mutex_test PUBLIC ${CMAKE_SOURCE_DIR})
target_link_libraries(Mutex_test Base::lib)