project(TmuduoTest)

add_executable(UnitTest unit_tests.cc ${CMAKE_SOURCE_DIR}/base/Palindrome.cc)
target_include_directories(UnitTest PUBLIC ${CMAKE_SOURCE_DIR})

target_link_libraries(UnitTest ${CONAN_LIBS})
add_test(NAME UnitTest COMMAND UnitTest)