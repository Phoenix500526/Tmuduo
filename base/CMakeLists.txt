project(TmuduoBase)

set(BASE_SRC 
    CountDownLatch.cc
    CurrentThread.cc
    Thread.cc
    LogStream.cc
    Logging.cc
    Exception.cc
    FileUtil.cc
    ProcessInfo.cc
    LogFile.cc
    AsyncLogging.cc
    Exception.cc
    ThreadPool.cc)

option(TIME_CHRONO "Use C++11 chrono library" OFF)

if(TIME_CHRONO)
    #set(TIME_SRC Timestamp_chrono.cc)
    message(WARNING "Timestamp_chrono has not defined")
else()
    set(TIME_SRC Timestamp.cc TimeZone.cc Date.cc)
endif()

add_library(${PROJECT_NAME} STATIC ${BASE_SRC} ${TIME_SRC})
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_SOURCE_DIR})
target_link_libraries(${PROJECT_NAME} PRIVATE Threads::Threads)

add_library(Base::lib ALIAS ${PROJECT_NAME})