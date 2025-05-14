#include "network_utils.h"

std::string NetworkUtils::sendRequest(const std::string& host, 
                                    int port, 
                                    const std::string& request,
                                    int timeout) {
    QTcpSocket socket;
    
    LOG_DEBUG("Connecting to " + host + ":" + std::to_string(port));
    socket.connectToHost(QString::fromStdString(host), port);
    
    if (!socket.waitForConnected(timeout)) {
        const QString error = socket.errorString();
        LOG_ERROR("Connection failed: " + error.toStdString());
        throw NetworkException("Connection failed: " + error.toStdString());
    }

    LOG_DEBUG("Sending request: " + request);
    socket.write(request.c_str(), request.size());
    if (!socket.waitForBytesWritten(timeout)) {
        const QString error = socket.errorString();
        LOG_ERROR("Failed to send data: " + error.toStdString());
        throw NetworkException("Failed to send data: " + error.toStdString());
    }

    if (!socket.waitForReadyRead(timeout)) {
        const QString error = socket.errorString();
        LOG_ERROR("No response from server: " + error.toStdString());
        throw NetworkException("No response from server: " + error.toStdString());
    }

    QByteArray response = socket.readAll();
    LOG_DEBUG("Received response: " + response.toStdString());
    return std::string(response.constData(), response.size());
}
