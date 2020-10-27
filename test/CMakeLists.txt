project(TmuduoTest)

add_executable(CountDownLatch_test CountDownLatch_test.cc)
target_include_directories(CountDownLatch_test PUBLIC ${CMAKE_SOURCE_DIR})
target_link_libraries(CountDownLatch_test Base::lib)


add_executable(Mutex_test Mutex_test.cc)
target_include_directories(Mutex_test PUBLIC ${CMAKE_SOURCE_DIR})
target_link_libraries(Mutex_test Base::lib)


add_executable(Exception_test Exception_test.cc)
target_include_directories(Exception_test PUBLIC ${CMAKE_SOURCE_DIR})
target_link_libraries(Exception_test Base::lib)

add_executable(Thread_test Thread_test.cc)
target_include_directories(Thread_test PUBLIC ${CMAKE_SOURCE_DIR})
target_link_libraries(Thread_test Base::lib)

add_executable(LogStream_Unittest LogStream_Unittest.cc)
target_include_directories(LogStream_Unittest PUBLIC ${CMAKE_SOURCE_DIR})
target_link_libraries(LogStream_Unittest Base::lib ${CONAN_LIBS})
add_test(NAME LogStream_Unittest COMMAND LogStream_Unittest)

if(TIME_CHRONO)
    message(WARNING "Timestamp_chrono_test.cc has not found")
else()
    add_executable(Timestame_Unittest Timestame_Unittest.cc)
    target_include_directories(Timestame_Unittest PUBLIC ${CMAKE_SOURCE_DIR})
    target_link_libraries(Timestame_Unittest Base::lib)
    add_test(NAME Timestame_Unittest COMMAND Timestame_Unittest)

    add_executable(TimeZone_Unittest TimeZone_Unittest.cc)
    target_include_directories(TimeZone_Unittest PUBLIC ${CMAKE_SOURCE_DIR})
    target_link_libraries(TimeZone_Unittest Base::lib)
    add_test(NAME TimeZone_Unittest COMMAND TimeZone_Unittest)

    add_executable(Date_Unittest Date_Unittest.cc)
    target_include_directories(Date_Unittest PUBLIC ${CMAKE_SOURCE_DIR})
    target_link_libraries(Date_Unittest Base::lib)
    add_test(NAME Date_Unittest COMMAND Date_Unittest)
endif()

add_executable(FileUtil_test FileUtil_test.cc)
target_include_directories(FileUtil_test PUBLIC ${CMAKE_SOURCE_DIR})
target_link_libraries(FileUtil_test Base::lib)
add_test(NAME FileUtil_test COMMAND FileUtil_test)

add_executable(ProcessInfo_test ProcessInfo_test.cc)
target_include_directories(ProcessInfo_test PUBLIC ${CMAKE_SOURCE_DIR})
target_link_libraries(ProcessInfo_test Base::lib)

add_executable(LogFile_test LogFile_test.cc)
target_include_directories(LogFile_test PUBLIC ${CMAKE_SOURCE_DIR})
target_link_libraries(LogFile_test Base::lib)

add_executable(AsyncLogging_test AsyncLogging_test.cc)
target_include_directories(AsyncLogging_test PUBLIC ${CMAKE_SOURCE_DIR})
target_link_libraries(AsyncLogging_test Base::lib)

add_executable(SyncQueue_test SyncQueue_test.cc)
target_include_directories(SyncQueue_test PUBLIC ${CMAKE_SOURCE_DIR})
target_link_libraries(SyncQueue_test Base::lib)

add_executable(ThreadPool_test ThreadPool_test.cc)
target_include_directories(ThreadPool_test PUBLIC ${CMAKE_SOURCE_DIR})
target_link_libraries(ThreadPool_test Base::lib)