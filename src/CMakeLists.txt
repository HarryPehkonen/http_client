# ./src/CMakeLists.txt
add_library(http_client_curl
    curl/curl_http_client.cpp
)

target_include_directories(http_client_curl
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(http_client_curl
    PUBLIC
        CURL::libcurl
        spdlog::spdlog
)
