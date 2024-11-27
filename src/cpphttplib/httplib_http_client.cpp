// src/cpphttplib/httplib_http_client.cpp
#include "http_client/httplib_http_client.hpp"
#include "http_client/exceptions.hpp"
#include <spdlog/spdlog.h>
#include <regex>

namespace http_client {

HttplibHTTPClient::HttplibHTTPClient() : m_timeout(30000) {}

std::future<HTTPResponse> HttplibHTTPClient::Get(const std::string& uri, const std::vector<std::string>& headers) {
    return PerformRequest("GET", uri, "", headers);
}

std::future<HTTPResponse> HttplibHTTPClient::Put(const std::string& uri, const std::string& body, const std::vector<std::string>& headers) {
    return PerformRequest("PUT", uri, body, headers);
}

std::future<HTTPResponse> HttplibHTTPClient::Post(const std::string& uri, const std::string& body, const std::vector<std::string>& headers) {
    return PerformRequest("POST", uri, body, headers);
}

std::future<HTTPResponse> HttplibHTTPClient::Patch(const std::string& uri, const std::string& body, const std::vector<std::string>& headers) {
    return PerformRequest("PATCH", uri, body, headers);
}

std::future<HTTPResponse> HttplibHTTPClient::Delete(const std::string& uri, const std::vector<std::string>& headers) {
    return PerformRequest("DELETE", uri, "", headers);
}

void HttplibHTTPClient::SetTimeout(std::chrono::milliseconds timeout) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_timeout = timeout;
}

std::chrono::milliseconds HttplibHTTPClient::GetTimeout() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_timeout;
}

std::pair<std::string, std::string> HttplibHTTPClient::ParseURI(const std::string& uri) {
    std::regex uri_regex(R"((https?:\/\/)?([^\/\s]+)(\/.*)?)", std::regex::icase);
    std::smatch matches;
    
    if (!std::regex_match(uri, matches, uri_regex)) {
        throw HTTPException("Invalid URI format");
    }
    
    std::string host = matches[1].str() + matches[2].str(); // protocol + host
    std::string path = matches[3].length() > 0 ? matches[3].str() : "/";
    
    return {host, path};
}

httplib::Client* HttplibHTTPClient::CreateClient(const std::string& host) {
    auto client = new httplib::Client(host);
    client->set_connection_timeout(static_cast<double>(m_timeout.count()) / 1000.0);
    client->set_read_timeout(static_cast<double>(m_timeout.count()) / 1000.0);
    client->set_write_timeout(static_cast<double>(m_timeout.count()) / 1000.0);
    return client;
}

std::future<HTTPResponse> HttplibHTTPClient::PerformRequest(
    const std::string& method, const std::string& uri, 
    const std::string& body, const std::vector<std::string>& headers) {
    
    return std::async(std::launch::async, [this, method, uri, body, headers]() {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto [host, path] = ParseURI(uri);
        
        std::unique_ptr<httplib::Client> client(CreateClient(host));
        httplib::Headers httplib_headers;
        
        // Convert headers to httplib format
        for (const auto& header : headers) {
            size_t colon_pos = header.find(':');
            if (colon_pos != std::string::npos) {
                std::string key = header.substr(0, colon_pos);
                std::string value = header.substr(colon_pos + 1);
                // Trim whitespace
                value = value.substr(value.find_first_not_of(" "));
                httplib_headers.emplace(key, value);
            }
        }

        HTTPResponse response;

        auto handle_result = [&](const httplib::Result& res) {
            if (res.error() != httplib::Error::Success) {
                if (res.error() == httplib::Error::Connection) {
                    throw ConnectionException("Failed to connect to server");
                } else if (res.error() == httplib::Error::Read) {
                    throw TimeoutException("Request timed out");
                } else {
                    throw HTTPException("Request failed with error code: " + std::to_string(static_cast<int>(res.error())));
                }
            }

            response.statusCode = res->status;
            response.body = res->body;
            
            // Convert response headers
            for (const auto& [key, value] : res->headers) {
                response.headers.push_back(key + ": " + value);
            }
        };
        
        if (method == "GET") {
            handle_result(client->Get(path.c_str(), httplib_headers));
        } else if (method == "POST") {
            handle_result(client->Post(path.c_str(), httplib_headers, body, "application/json"));
        } else if (method == "PUT") {
            handle_result(client->Put(path.c_str(), httplib_headers, body, "application/json"));
        } else if (method == "DELETE") {
            handle_result(client->Delete(path.c_str(), httplib_headers));
        } else if (method == "PATCH") {
            handle_result(client->Patch(path.c_str(), httplib_headers, body, "application/json"));
        } else {
            throw HTTPException("Unsupported HTTP method: " + method);
        }

        return response;
    });
}

} // namespace http_client
