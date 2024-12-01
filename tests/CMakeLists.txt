find_package(GTest REQUIRED)
find_package(Threads REQUIRED)

# Find Deno for test server
find_program(DENO_EXECUTABLE deno REQUIRED)
message(STATUS "Found Deno: ${DENO_EXECUTABLE}")

# Add nlohmann_json
include(FetchContent)
FetchContent_Declare(json
    URL https://github.com/nlohmann/json/releases/download/v3.11.2/json.tar.xz
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
FetchContent_MakeAvailable(json)

# Create test executable
add_executable(http_client_tests
    test_main.cpp
    http_client_tests.cpp
)

# Configure the test executable
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

# Add compile definition to inform tests which backend is being used
if(HTTP_CLIENT_BACKEND STREQUAL "CURL")
    target_compile_definitions(http_client_tests PRIVATE HTTP_CLIENT_BACKEND=CURL)
elseif(HTTP_CLIENT_BACKEND STREQUAL "HTTPLIB")
    target_compile_definitions(http_client_tests PRIVATE HTTP_CLIENT_BACKEND=HTTPLIB)
endif()

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

add_test(NAME http_client_tests COMMAND http_client_tests)
