cmake_minimum_required(VERSION 3.14)
project(http_client VERSION 1.0.0 LANGUAGES CXX)

include(FetchContent)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Backend selection option
set(HTTP_CLIENT_BACKEND "CURL" CACHE STRING "Backend to use for HTTP client (CURL or HTTPLIB)")
set_property(CACHE HTTP_CLIENT_BACKEND PROPERTY STRINGS CURL HTTPLIB)

message(STATUS "Selected HTTP_CLIENT_BACKEND: ${HTTP_CLIENT_BACKEND}")

# Common dependencies
find_package(spdlog REQUIRED)
find_package(Threads REQUIRED)
find_package(OpenSSL REQUIRED)

# Backend-specific setup
if(HTTP_CLIENT_BACKEND STREQUAL "CURL")
    find_package(CURL REQUIRED)
    message(STATUS "CURL found: ${CURL_FOUND}")
    message(STATUS "CURL libraries: ${CURL_LIBRARIES}")
    
    set(HTTP_CLIENT_SOURCES
        src/curl/curl_http_client.cpp
    )
    set(HTTP_CLIENT_LIBRARIES 
        CURL::libcurl
    )
elseif(HTTP_CLIENT_BACKEND STREQUAL "HTTPLIB")
    # Fetch and configure cpp-httplib
    FetchContent_Declare(
        cpp-httplib
        GIT_REPOSITORY https://github.com/yhirose/cpp-httplib.git
        GIT_TAG v0.14.1
    )

    FetchContent_GetProperties(cpp-httplib)
    if(NOT cpp-httplib_POPULATED)
        FetchContent_Populate(cpp-httplib)
        
        set(HTTPLIB_COMPILE ON CACHE INTERNAL "")
        set(HTTPLIB_REQUIRE_OPENSSL ON CACHE INTERNAL "")
        set(BUILD_SHARED_LIBS OFF CACHE INTERNAL "")
        
        add_subdirectory(${cpp-httplib_SOURCE_DIR} ${cpp-httplib_BINARY_DIR})
    endif()

    set(HTTP_CLIENT_SOURCES
        src/cpphttplib/httplib_http_client.cpp
    )
    set(HTTP_CLIENT_LIBRARIES 
        httplib
        OpenSSL::SSL 
        OpenSSL::Crypto
    )
endif()

message(STATUS "HTTP_CLIENT_SOURCES: ${HTTP_CLIENT_SOURCES}")
message(STATUS "HTTP_CLIENT_LIBRARIES: ${HTTP_CLIENT_LIBRARIES}")

# Create the library target
add_library(http_client STATIC ${HTTP_CLIENT_SOURCES})

# Set include directories
target_include_directories(http_client
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE
        ${cpp-httplib_SOURCE_DIR}
)

# Link dependencies
target_link_libraries(http_client
    PUBLIC
        ${HTTP_CLIENT_LIBRARIES}
        spdlog::spdlog
        Threads::Threads
)

# Add compile definitions
target_compile_definitions(http_client
    PRIVATE
        CPPHTTPLIB_OPENSSL_SUPPORT
        CPPHTTPLIB_USE_CERTS_FROM_SYSTEM
)

# Optional: Enable testing
option(BUILD_TESTING "Build tests" OFF)
if(BUILD_TESTING)
    enable_testing()
    add_subdirectory(tests)
endif()
