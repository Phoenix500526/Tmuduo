project(TmuduoPubsub)

add_executable(Hub hub.cc codec.cc)
target_include_directories(Hub PUBLIC ${CMAKE_SOURCE_DIR})
target_link_libraries(Hub Net::lib Inspect::lib)

add_library(${PROJECT_NAME} STATIC pubsub.cc codec.cc)
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_SOURCE_DIR})
target_link_libraries(${PROJECT_NAME} PRIVATE Net::lib)
add_library(Pubsub::lib ALIAS ${PROJECT_NAME})

add_executable(Pub pub.cc)
target_include_directories(Pub PUBLIC ${CMAKE_SOURCE_DIR})
target_link_libraries(Pub Net::lib Pubsub::lib)

add_executable(Sub sub.cc)
target_include_directories(Sub PUBLIC ${CMAKE_SOURCE_DIR})
target_link_libraries(Sub Net::lib Pubsub::lib)