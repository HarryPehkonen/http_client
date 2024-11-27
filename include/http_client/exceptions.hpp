// ./include/http_client/exceptions.hpp
#ifndef HTTP_CLIENT_EXCEPTIONS_HPP
#define HTTP_CLIENT_EXCEPTIONS_HPP

#include <stdexcept>
#include <string>

namespace http_client {

class HTTPException : public std::runtime_error {
public:
    explicit HTTPException(const std::string& message) 
        : std::runtime_error(message) {}
};

class TimeoutException : public HTTPException {
public:
    explicit TimeoutException(const std::string& message) 
        : HTTPException(message) {}
};

class ConnectionException : public HTTPException {
public:
    explicit ConnectionException(const std::string& message) 
        : HTTPException(message) {}
};

class InvalidResponseException : public HTTPException {
public:
    explicit InvalidResponseException(const std::string& message) 
        : HTTPException(message) {}
};

} // namespace http_client

#endif // HTTP_CLIENT_EXCEPTIONS_HPP
