project(TmuduoInspect)

set(INSPECT_SRCS
    SystemInspector.cc
    ProcessInspector.cc
    PerformanceInspector.cc
    Inspector.cc)

add_library(${PROJECT_NAME} STATIC ${INSPECT_SRCS})
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_SOURCE_DIR})
target_link_libraries(${PROJECT_NAME} PRIVATE Net::lib HTTP::lib)
add_library(Inspect::lib ALIAS ${PROJECT_NAME})

add_executable(Inspector_test tests/inspector_test.cc)
target_include_directories(Inspector_test PUBLIC ${CMAKE_SOURCE_DIR})
target_link_libraries(Inspector_test Net::lib Inspect::lib)