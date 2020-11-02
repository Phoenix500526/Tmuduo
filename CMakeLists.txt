cmake_minimum_required(VERSION 3.13)
project(Tmuduo CXX)

file(GLOB CLANG_FORMAT_BIN_NAME /usr/bin/clang-format-[3-9]*)

if(NOT CLANG_FORMAT_BIN_NAME-NOTFOUND)
    message("The clang-format is ${CLANG_FORMAT_BIN_NAME}")
    file(GLOB_RECURSE ALL_SOURCE_FILES *.cc *.h)
    add_custom_target(
            clang-format
            COMMAND ${CLANG_FORMAT_BIN_NAME}
            -style=google
            -i
            ${ALL_SOURCE_FILES}
    )
endif()

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()
enable_testing()

set(CMAKE_CXX_STANDARD 11)
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    add_compile_options(-Wthread-safety )
endif()

add_compile_options(
 # -DVALGRIND
 -Wall
 -Wno-unused-parameter
 -Woverloaded-virtual
 -Wpointer-arith
 -Wwrite-strings
 -O3
 )

add_link_options(-rdynamic)

add_subdirectory(test)
add_subdirectory(base)
add_subdirectory(net)
add_subdirectory(examples)