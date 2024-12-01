#include "http_client/curl_http_client.hpp"
#include "http_client/httplib_http_client.hpp"
#include "http_client/exceptions.hpp"
#include <gtest/gtest.h>
#include <nlohmann/json.hpp>
#include <thread>
#include <memory>

using json = nlohmann::json;

// Factory function to create client implementation
std::unique_ptr<http_client::HTTPClient> CreateClient() {
#if defined(HTTP_CLIENT_BACKEND_CURL)
    return std::make_unique<http_client::CurlHTTPClient>();
#elif defined(HTTP_CLIENT_BACKEND_HTTPLIB)
    return std::make_unique<http_client::HttplibHTTPClient>();
#else
    #error "No HTTP_CLIENT_BACKEND defined"
#endif
}

class HTTPClientTest : public ::testing::Test {
protected:
    void SetUp() override {
        client = CreateClient();
        baseUrl = "http://localhost:8080";
    }

    void TearDown() override {
        client.reset();
    }

    std::unique_ptr<http_client::HTTPClient> client;
    std::string baseUrl;
};

TEST_F(HTTPClientTest, GetRequest) {
    auto future = client->Get(baseUrl + "/test", {"Content-Type: application/json"});
    auto response = future.get();
    EXPECT_EQ(200, response.statusCode);

    auto json_response = json::parse(response.body);
    EXPECT_EQ("success", json_response["status"]);
    EXPECT_EQ("GET response", json_response["message"]);
}

TEST_F(HTTPClientTest, PostRequest) {
    std::string testBody = "{\"key\":\"value\"}";
    auto future = client->Post(
        baseUrl + "/test",
        testBody,
        {"Content-Type: application/json"}
    );
    auto response = future.get();
    EXPECT_EQ(201, response.statusCode);

    auto json_response = json::parse(response.body);
    EXPECT_EQ("success", json_response["status"]);
    EXPECT_EQ("POST response", json_response["message"]);
    EXPECT_EQ("value", json_response["received"]["key"]);
}

TEST_F(HTTPClientTest, PutRequest) {
    std::string testBody = "{\"key\":\"updated\"}";
    auto future = client->Put(
        baseUrl + "/test",
        testBody,
        {"Content-Type: application/json"}
    );
    auto response = future.get();
    EXPECT_EQ(200, response.statusCode);

    auto json_response = json::parse(response.body);
    EXPECT_EQ("success", json_response["status"]);
    EXPECT_EQ("PUT response", json_response["message"]);
    EXPECT_EQ("updated", json_response["received"]["key"]);
}

TEST_F(HTTPClientTest, DeleteRequest) {
    auto future = client->Delete(baseUrl + "/test");
    auto response = future.get();
    EXPECT_EQ(200, response.statusCode);

    auto json_response = json::parse(response.body);
    EXPECT_EQ("success", json_response["status"]);
    EXPECT_EQ("DELETE response", json_response["message"]);
}

TEST_F(HTTPClientTest, Timeout) {
    client->SetTimeout(std::chrono::milliseconds(100));
    EXPECT_EQ(std::chrono::milliseconds(100), client->GetTimeout());
}

TEST_F(HTTPClientTest, ConnectionError) {
    EXPECT_THROW({
        auto future = client->Get("http://localhost:12345/nonexistent");
        future.get();
    }, http_client::ConnectionException);
}

TEST_F(HTTPClientTest, Headers) {
    auto future = client->Get(
        baseUrl + "/headers",
        {
            "X-Custom-Header: test-value",
            "Another-Header: another-value"
        }
    );
    auto response = future.get();
    EXPECT_EQ(200, response.statusCode);

    auto json_response = json::parse(response.body);
    EXPECT_EQ("test-value", json_response["headers"]["x-custom-header"]);
    EXPECT_EQ("another-value", json_response["headers"]["another-header"]);
}

TEST_F(HTTPClientTest, LargePayload) {
    // Generate a large JSON payload
    json large_payload;
    for (int i = 0; i < 1000; i++) {
        large_payload[std::to_string(i)] = std::string(100, 'a');
    }
    
    std::string testBody = large_payload.dump();
    auto future = client->Post(
        baseUrl + "/echo",
        testBody,
        {"Content-Type: application/json"}
    );
    auto response = future.get();
    EXPECT_EQ(200, response.statusCode);

    auto json_response = json::parse(response.body);
    EXPECT_EQ(large_payload, json_response["received"]);
}
