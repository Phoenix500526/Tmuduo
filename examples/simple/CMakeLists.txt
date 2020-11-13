add_executable(Echo echo/echo.cc echo/main.cc)
target_include_directories(Echo PUBLIC ${CMAKE_SOURCE_DIR})
target_link_libraries(Echo Base::lib Net::lib)