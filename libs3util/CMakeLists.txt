cmake_minimum_required (VERSION 3.12)

find_package(OpenSSL REQUIRED)
find_package(CURL REQUIRED)

include_directories(${CURL_INCLUDE_DIRS} ${OPENSSL_INCLUDE_DIR} )
set(LIBS ${OPENSSL_LIBRARIES} ${CURL_LIBRARIES})

add_library(s3util s3util.c)

set_target_properties(s3util PROPERTIES PUBLIC_HEADER "s3util.h")

target_link_libraries(s3util PRIVATE ${LIBS})

install(TARGETS s3util LIBRARY DESTINATION lib PUBLIC_HEADER DESTINATION include)

