cmake_minimum_required(VERSION 3.5)
project(Tmuduo CXX)

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()
enable_testing()

set(CMAKE_CXX_STANDARD 11)


add_subdirectory(test)
add_executable(${PROJECT_NAME} main.cc)