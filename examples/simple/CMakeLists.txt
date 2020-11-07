add_executable(echo echo/echo.cc echo/main.cc)
target_include_directories(echo PUBLIC ${CMAKE_SOURCE_DIR})
target_link_libraries(echo Base::lib Net::lib)