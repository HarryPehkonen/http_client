#include <gtest/gtest.h>
#include <cstdlib>
#include <string>
#include <thread>
#include <chrono>
#include <memory>
#include <fstream>
#include <filesystem>
#include "spdlog/spdlog.h"
#include "config.hpp"

class TestEnvironment : public ::testing::Environment {
public:
    TestEnvironment() {
        // Configure logging for test environment
        spdlog::set_level(spdlog::level::debug);
    }

    void SetUp() override {
        spdlog::info("Starting test server...");
        
        std::filesystem::path test_server_path = std::filesystem::path(BUILD_DIR) / "tests" / "test_server.ts";
        const std::string server_command = "deno run --allow-net " + test_server_path.string();
        
        #ifdef _WIN32
        server_process_ = _popen(server_command.c_str(), "r");
        #else
        // Start server in background and save PID
        server_process_ = popen((server_command + " & echo $! > server.pid").c_str(), "r");
        #endif
        
        if (!server_process_) {
            throw std::runtime_error("Failed to start test server");
        }

        // Allow server startup time
        std::this_thread::sleep_for(std::chrono::seconds(2));
        spdlog::info("Test server started");
    }

    void TearDown() override {
        spdlog::info("Stopping test server...");
        if (server_process_) {
            #ifdef _WIN32
            _pclose(server_process_);
            #else
            // Send SIGINT to allow graceful shutdown
            std::ifstream pid_file;
            pid_file.open("server.pid");
            if (pid_file.is_open()) {
                std::string pid;
                std::getline(pid_file, pid);
                pid_file.close();
                if (!pid.empty()) {
                    system(("kill " + pid).c_str());
                    std::this_thread::sleep_for(std::chrono::seconds(2));
                }
            }
            pclose(server_process_);
            #endif
            spdlog::info("Test server stopped");
        }
    }

private:
    FILE* server_process_ = nullptr;
};

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    ::testing::AddGlobalTestEnvironment(new TestEnvironment);
    return RUN_ALL_TESTS();
}
