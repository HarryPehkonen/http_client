find_package(GTest REQUIRED)
find_package(Threads REQUIRED)

message(STATUS "Configuring tests for backend: ${HTTP_CLIENT_BACKEND}")

# Select test file based on backend
if(HTTP_CLIENT_BACKEND STREQUAL "CURL")
    set(TEST_SOURCE curl_client_tests.cpp)
    set(TEST_LIBS
        http_client
        spdlog::spdlog
        GTest::GTest
        GTest::Main
        Threads::Threads
        CURL::libcurl
    )
elseif(HTTP_CLIENT_BACKEND STREQUAL "HTTPLIB")
    set(TEST_SOURCE httplib_client_tests.cpp)
    set(TEST_LIBS
        http_client
        httplib
        spdlog::spdlog
        GTest::GTest
        GTest::Main
        Threads::Threads
        OpenSSL::SSL
        OpenSSL::Crypto
    )
else()
    message(FATAL_ERROR "Unknown HTTP_CLIENT_BACKEND: ${HTTP_CLIENT_BACKEND}")
endif()

message(STATUS "Using test source: ${TEST_SOURCE}")
message(STATUS "Using test libraries: ${TEST_LIBS}")

add_executable(http_client_tests ${TEST_SOURCE})

target_include_directories(http_client_tests
    PRIVATE
        ${CMAKE_SOURCE_DIR}/include
)

# Add backend-specific compile definitions
if(HTTP_CLIENT_BACKEND STREQUAL "HTTPLIB")
    target_compile_definitions(http_client_tests
        PRIVATE
            CPPHTTPLIB_OPENSSL_SUPPORT
            CPPHTTPLIB_USE_CERTS_FROM_SYSTEM
    )
endif()

target_link_libraries(http_client_tests
    PRIVATE
        ${TEST_LIBS}
)

add_test(NAME http_client_tests COMMAND http_client_tests)
