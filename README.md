# http_client
Future-returning HTTP client implementations with different options for back end

## Usage

```cmake
include(FetchContent)

# Fetch the HTTP client library
FetchContent_Declare(
    http_client
    GIT_REPOSITORY https://github.com/HarryPehkonen/http_client.git
    GIT_TAG main  # or specify a specific commit/tag
)

# Choose the backend before making the content available
set(HTTP_CLIENT_BACKEND "HTTPLIB" CACHE STRING "Backend to use for HTTP client (CURL or HTTPLIB)")
set(BUILD_TESTING OFF)  # Disable testing of the HTTP client library

FetchContent_MakeAvailable(http_client)

# Your project's target
add_executable(your_project main.cpp)
target_link_libraries(your_project
    PRIVATE
        http_client
)
```

### Using the HTTP client

```cpp
#include <http_client/http_client.hpp>

int main() {
    http_client::HttpClient client;

    std::future<http_client::HTTP_Response> future = client.Get("https://example.com");
    auto response = future.get();
    if (response) {
        std::cout << response->status_code << std::endl;
        std::cout << response->body << std::endl;
    }

    return 0;
}
```

There is a working example in ./http_client_test.  To build the example, run the following commands:

```cmake
mkdir build
cd build
cmake -DHTTP_CLIENT_BACKEND=HTTPLIB ..
cmake --build .
```

