cmake_minimum_required(VERSION 3.5)
project(Tmuduo CXX)

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()
enable_testing()

set(CMAKE_CXX_STANDARD 11)

find_program(CLANG_FORMAT_BIN_NAME NAMES clang-format-3.8 PATH /usr/bin/)

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


add_subdirectory(test)
add_subdirectory(base)