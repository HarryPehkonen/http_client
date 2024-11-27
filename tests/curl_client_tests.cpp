// tests/curl_client_tests.cpp
#include <gtest/gtest.h>
#include <thread>
#include <memory>
#include "http_client/curl_http_client.hpp"
#include "http_client/exceptions.hpp"

// Simple HTTP server for testing
class TestServer {
public:
    TestServer() {
        // In a real implementation, we'd set up a basic HTTP server
        // For now, we'll just simulate the server
    }
    
    ~TestServer() {
        // Cleanup
    }
};

class CurlClientTest : public ::testing::Test {
protected:
    void SetUp() override {
        server = std::make_unique<TestServer>();
        client = std::make_unique<http_client::CurlHTTPClient>();
        baseUrl = "http://localhost:8080";
    }

    void TearDown() override {
        client.reset();
        server.reset();
    }

    std::unique_ptr<TestServer> server;
    std::unique_ptr<http_client::CurlHTTPClient> client;
    std::string baseUrl;
};

TEST_F(CurlClientTest, GetRequest) {
    auto future = client->Get(baseUrl + "/test", {"X-Test: test-value"});
    auto response = future.get();
    EXPECT_EQ(200, response.statusCode);
}

TEST_F(CurlClientTest, PostRequest) {
    std::string testBody = "{\"key\":\"value\"}";
    auto future = client->Post(
        baseUrl + "/test",
        testBody,
        {"Content-Type: application/json"}
    );
    auto response = future.get();
    EXPECT_EQ(201, response.statusCode);
}

TEST_F(CurlClientTest, PutRequest) {
    std::string testBody = "{\"key\":\"updated\"}";
    auto future = client->Put(
        baseUrl + "/test",
        testBody,
        {"Content-Type: application/json"}
    );
    auto response = future.get();
    EXPECT_EQ(200, response.statusCode);
}

TEST_F(CurlClientTest, DeleteRequest) {
    auto future = client->Delete(baseUrl + "/test");
    auto response = future.get();
    EXPECT_EQ(200, response.statusCode);
}

TEST_F(CurlClientTest, Timeout) {
    client->SetTimeout(std::chrono::milliseconds(100));
    EXPECT_EQ(std::chrono::milliseconds(100), client->GetTimeout());
}

TEST_F(CurlClientTest, ConnectionError) {
    EXPECT_THROW({
        auto future = client->Get("http://localhost:12345/nonexistent");
        future.get();
    }, http_client::ConnectionException);
}
