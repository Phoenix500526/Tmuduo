project(TmuduoBase)

set(BASE_SRC 
    CountDownLatch.cc
    CurrentThread.cc
    Thread.cc
    LogStream.cc
    Logging.cc
    Exception.cc
    Timestamp.cc
    Date.cc)

add_library(${PROJECT_NAME} STATIC ${BASE_SRC})
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_SOURCE_DIR})
target_link_libraries(${PROJECT_NAME} PRIVATE Threads::Threads)

add_library(Base::lib ALIAS ${PROJECT_NAME})