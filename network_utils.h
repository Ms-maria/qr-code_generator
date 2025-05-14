#ifndef NETWORK_UTILS_H
#define NETWORK_UTILS_H

#include <string>
#include <stdexcept>
#include <QTcpSocket>
#include "logging.h"

class NetworkException : public std::runtime_error {
public:
    explicit NetworkException(const std::string& msg) 
        : std::runtime_error("[NETWORK ERROR] " + msg) {}
};

class NetworkUtils {
public:
    static std::string sendRequest(const std::string& host, 
                                 int port, 
                                 const std::string& request,
                                 int timeout = 5000);
};

#endif // NETWORK_UTILS_H
