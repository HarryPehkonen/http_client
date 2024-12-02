# CMakeLists.txt
cmake_minimum_required(VERSION 3.14)
project(http_client_example VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

# Fetch the HTTP client library
FetchContent_Declare(
    http_client
    GIT_REPOSITORY https://github.com/HarryPehkonen/http_client.git
    GIT_TAG main  # or specify a specific commit/tag
)

# Choose the backend before making the content available
set(HTTP_CLIENT_BACKEND "CURL" CACHE STRING "Backend to use for HTTP client (CURL or HTTPLIB)")
set(BUILD_TESTING OFF)  # Disable testing of the HTTP client library

FetchContent_MakeAvailable(http_client)

# Create the example executable
add_executable(http_client_example main.cpp)

target_link_libraries(http_client_example
    PRIVATE
        http_client
)
