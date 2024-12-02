// main.cpp
#include "http_client/curl_http_client.hpp"
#include "http_client/httplib_http_client.hpp"
#include <iostream>
#include <memory>
#include <string>

std::unique_ptr<http_client::HTTPClient> create_client() {
#if defined(HTTP_CLIENT_BACKEND_CURL)
    return std::make_unique<http_client::CurlHTTPClient>();
#elif defined(HTTP_CLIENT_BACKEND_HTTPLIB)
    return std::make_unique<http_client::HttplibHTTPClient>();
#else
    #error "No HTTP_CLIENT_BACKEND defined"
#endif
}

int main() {
    try {
        auto client = create_client();
        client->SetTimeout(std::chrono::milliseconds(5000));

        // Make a GET request to httpbin
        std::cout << "Making GET request...\n";
        auto future = client->Get("http://httpbin.org/get", 
            {"Accept: application/json"});
        
        auto response = future.get();
        std::cout << "Status code: " << response.statusCode << "\n";
        std::cout << "Response body:\n" << response.body << "\n";

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
