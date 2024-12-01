cmake_minimum_required(VERSION 3.14)
project(http_client VERSION 1.0.0 LANGUAGES CXX)

include(FetchContent)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Find common dependencies
find_package(spdlog REQUIRED)
find_package(Threads REQUIRED)

# Backend selection option
set(HTTP_CLIENT_BACKEND "CURL" CACHE STRING "Backend to use for HTTP client (CURL or HTTPLIB)")
set_property(CACHE HTTP_CLIENT_BACKEND PROPERTY STRINGS CURL HTTPLIB)

message(STATUS "Selected HTTP_CLIENT_BACKEND: ${HTTP_CLIENT_BACKEND}")

# Define the core library without backend-specific code
add_library(http_client_core INTERFACE)
target_include_directories(http_client_core 
    INTERFACE 
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Backend-specific setup
if(HTTP_CLIENT_BACKEND STREQUAL "CURL")
    find_package(CURL REQUIRED)
    add_library(http_client STATIC 
        src/curl/curl_http_client.cpp
    )
    target_compile_definitions(http_client 
        PUBLIC 
            HTTP_CLIENT_BACKEND_CURL
    )
    target_link_libraries(http_client
        PUBLIC
            http_client_core
            CURL::libcurl
            spdlog::spdlog
            Threads::Threads
    )

elseif(HTTP_CLIENT_BACKEND STREQUAL "HTTPLIB")
    find_package(OpenSSL REQUIRED)
    
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
    
    add_library(http_client STATIC 
        src/cpphttplib/httplib_http_client.cpp
    )
    target_compile_definitions(http_client 
        PUBLIC 
            HTTP_CLIENT_BACKEND_HTTPLIB
            CPPHTTPLIB_OPENSSL_SUPPORT
            CPPHTTPLIB_USE_CERTS_FROM_SYSTEM
    )
    target_include_directories(http_client
        PRIVATE
            ${cpp-httplib_SOURCE_DIR}
    )
    target_link_libraries(http_client
        PUBLIC
            http_client_core
            httplib
            OpenSSL::SSL
            OpenSSL::Crypto
            spdlog::spdlog
            Threads::Threads
    )
else()
    message(FATAL_ERROR "Invalid HTTP_CLIENT_BACKEND value: ${HTTP_CLIENT_BACKEND}")
endif()

# Optional: Enable testing
option(BUILD_TESTING "Build tests" OFF)
if(BUILD_TESTING)
    enable_testing()
    
    # Find test dependencies
    find_package(GTest REQUIRED)
    
    # Find Deno for test server
    find_program(DENO_EXECUTABLE deno REQUIRED)
    message(STATUS "Found Deno: ${DENO_EXECUTABLE}")

    # Add nlohmann_json for tests
    FetchContent_Declare(json
        URL https://github.com/nlohmann/json/releases/download/v3.11.2/json.tar.xz
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    )
    FetchContent_MakeAvailable(json)

    # Copy test server script
    configure_file(
        ${CMAKE_SOURCE_DIR}/tests/server/test_server.ts
        ${CMAKE_BINARY_DIR}/tests/test_server.ts
        COPYONLY
    )

    # Generate config header
    configure_file(
        ${CMAKE_SOURCE_DIR}/src/config.hpp.in
        ${CMAKE_BINARY_DIR}/generated/config.hpp
    )

    # Create test executable
    add_executable(http_client_tests
        tests/test_main.cpp
        tests/http_client_tests.cpp
    )

    target_include_directories(http_client_tests
        PRIVATE
            ${CMAKE_SOURCE_DIR}/include
            ${CMAKE_BINARY_DIR}/generated
    )

    target_link_libraries(http_client_tests
        PRIVATE
            http_client
            GTest::GTest
            GTest::Main
            nlohmann_json::nlohmann_json
    )

    add_test(NAME http_client_tests COMMAND http_client_tests)
endif()
