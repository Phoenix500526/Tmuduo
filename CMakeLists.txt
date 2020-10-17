cmake_minimum_required(VERSION 3.5)
project(Tmuduo CXX)

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()
enable_testing()

set(CMAKE_CXX_STANDARD 11)

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


add_subdirectory(test)
add_subdirectory(base)