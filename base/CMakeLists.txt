project(TmuduoBase)

set(BASE_SRC Palindrome.cc)

add_library(${PROJECT_NAME} STATIC ${BASE_SRC})
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_SOURCE_DIR})