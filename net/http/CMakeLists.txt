project(TmuduoHttp)

set(HTTP_SRC
    HttpResponse.cc
    HttpContext.cc
    HttpServer.cc)

add_library(${PROJECT_NAME} STATIC ${HTTP_SRC})
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_SOURCE_DIR})
add_library(HTTP::lib ALIAS ${PROJECT_NAME})

set(HTTP_HEADERS
    HttpContext.h
    HttpRequest.h
    HttpResponse.h
    HttpServer.h)
install(FILES ${HTTP_HEADERS} DESTINATION include/net/http)


add_executable(HttpRequest_Unittest tests/HttpRequest_Unittest.cc)
target_link_libraries(HttpRequest_Unittest HTTP::lib Net::lib ${CONAN_LIBS})
add_test(NAME HttpRequest_Unittest COMMAND HttpRequest_Unittest)

add_executable(HttpServer_test tests/HttpServer_test.cc)
target_link_libraries(HttpServer_test HTTP::lib Net::lib ${CONAN_LIBS})
