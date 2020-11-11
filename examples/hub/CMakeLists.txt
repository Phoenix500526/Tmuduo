project(TmuduoPubsub)

add_executable(hub hub.cc codec.cc)
target_include_directories(hub PUBLIC ${CMAKE_SOURCE_DIR})
target_link_libraries(hub Net::lib Inspect::lib)

add_library(${PROJECT_NAME} STATIC pubsub.cc codec.cc)
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_SOURCE_DIR})
target_link_libraries(${PROJECT_NAME} PRIVATE Net::lib)
add_library(Pubsub::lib ALIAS ${PROJECT_NAME})

add_executable(pub pub.cc)
target_include_directories(pub PUBLIC ${CMAKE_SOURCE_DIR})
target_link_libraries(pub Net::lib Pubsub::lib)

add_executable(sub sub.cc)
target_include_directories(sub PUBLIC ${CMAKE_SOURCE_DIR})
target_link_libraries(sub Net::lib Pubsub::lib)