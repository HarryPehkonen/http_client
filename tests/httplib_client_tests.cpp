// tests/httplib_client_tests.cpp
#include "http_client/httplib_http_client.hpp"
#include "http_client/exceptions.hpp"
#include "../src/cpphttplib/httplib_impl.hpp"  // Include the implementation
#include <gtest/gtest.h>
#include <thread>
#include <memory>

class MockHTTPServer {
public:
    MockHTTPServer() : server(std::make_unique<httplib::Server>()) {
        server->Get("/test", [this](const httplib::Request& req, httplib::Response& res) {
            res.status = 200;
            res.set_content("GET response", "text/plain");
            lastMethod = "GET";
            lastPath = req.path;
            lastHeaders = req.headers;
        });

        server->Post("/test", [this](const httplib::Request& req, httplib::Response& res) {
            res.status = 201;
            res.set_content("POST response", "text/plain");
            lastMethod = "POST";
            lastPath = req.path;
            lastBody = req.body;
            lastHeaders = req.headers;
        });

        serverThread = std::thread([this]() {
            server->listen("localhost", 8080);
        });
        
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    ~MockHTTPServer() {
        server->stop();
        if (serverThread.joinable()) {
            serverThread.join();
        }
    }

    std::string lastMethod;
    std::string lastPath;
    std::string lastBody;
    httplib::Headers lastHeaders;

private:
    std::unique_ptr<httplib::Server> server;
    std::thread serverThread;
};

class HttplibClientTest : public ::testing::Test {
protected:
    void SetUp() override {
        server = std::make_unique<MockHTTPServer>();
        client = std::make_unique<http_client::HttplibHTTPClient>();
        baseUrl = "http://localhost:8080";
    }

    void TearDown() override {
        client.reset();
        server.reset();
    }

    std::unique_ptr<MockHTTPServer> server;
    std::unique_ptr<http_client::HttplibHTTPClient> client;
    std::string baseUrl;
};

TEST_F(HttplibClientTest, GetRequest) {
    auto future = client->Get(baseUrl + "/test", {"X-Test: test-value"});
    auto response = future.get();
    EXPECT_EQ(200, response.statusCode);
    EXPECT_EQ("GET", server->lastMethod);
    EXPECT_EQ("/test", server->lastPath);
}

TEST_F(HttplibClientTest, PostRequest) {
    std::string testBody = "{\"key\":\"value\"}";
    auto future = client->Post(
        baseUrl + "/test",
        testBody,
        {"Content-Type: application/json"}
    );
    auto response = future.get();
    EXPECT_EQ(201, response.statusCode);
    EXPECT_EQ("POST", server->lastMethod);
    EXPECT_EQ(testBody, server->lastBody);
}

TEST_F(HttplibClientTest, Timeout) {
    client->SetTimeout(std::chrono::milliseconds(100));
    EXPECT_EQ(std::chrono::milliseconds(100), client->GetTimeout());
}

TEST_F(HttplibClientTest, ConnectionError) {
    EXPECT_THROW({
        auto future = client->Get("http://localhost:12345/nonexistent");
        future.get();
    }, http_client::ConnectionException);
}
