// ./include/http_client/httplib_http_client.hpp
#ifndef HTTP_CLIENT_HTTPLIB_HTTP_CLIENT_HPP
#define HTTP_CLIENT_HTTPLIB_HTTP_CLIENT_HPP

#include "http_client/http_client.hpp"
#include <httplib.h>
#include <mutex>

namespace http_client {

class HttplibHTTPClient : public HTTPClient {
public:
    HttplibHTTPClient();
    ~HttplibHTTPClient() override = default;

    std::future<HTTPResponse> Get(const std::string& uri, const std::vector<std::string>& headers = {}) override;
    std::future<HTTPResponse> Put(const std::string& uri, const std::string& body, const std::vector<std::string>& headers = {}) override;
    std::future<HTTPResponse> Post(const std::string& uri, const std::string& body, const std::vector<std::string>& headers = {}) override;
    std::future<HTTPResponse> Patch(const std::string& uri, const std::string& body, const std::vector<std::string>& headers = {}) override;
    std::future<HTTPResponse> Delete(const std::string& uri, const std::vector<std::string>& headers = {}) override;

    void SetTimeout(std::chrono::milliseconds timeout) override;
    std::chrono::milliseconds GetTimeout() const override;

private:
    std::future<HTTPResponse> PerformRequest(const std::string& method, const std::string& uri, 
                                           const std::string& body, const std::vector<std::string>& headers);
    static std::pair<std::string, std::string> ParseURI(const std::string& uri);

    httplib::Client* CreateClient(const std::string& host);
    std::chrono::milliseconds m_timeout;
    mutable std::mutex m_mutex;
};

} // namespace http_client

#endif // HTTP_CLIENT_HTTPLIB_HTTP_CLIENT_HPP
