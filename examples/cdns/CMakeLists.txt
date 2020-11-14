add_library(Tmuduo_cdns Resolver.cc)
target_include_directories(Tmuduo_cdns PUBLIC ${CMAKE_SOURCE_DIR})
target_link_libraries(Tmuduo_cdns Net::lib cares)
add_library(Cdns::lib ALIAS Tmuduo_cdns)

add_executable(Cdns dns.cc)
target_include_directories(Cdns PUBLIC ${CMAKE_SOURCE_DIR})
target_link_libraries(Cdns Cdns::lib)