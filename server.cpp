#include <iostream>
#include <string>
#include <thread>
#include <mutex>
#include <vector>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include "qr_generator.h"

std::mutex qr_mutex;
const int PORT = 8080;

void handle_client(int client_socket) {
    char buffer[1024] = {0};
    int bytes_read = read(client_socket, buffer, sizeof(buffer)-1);
    
    if (bytes_read <= 0) {
        close(client_socket);
        return;
    }
    
    std::string request(buffer, bytes_read);
    LOG_INFO("Received request: " + request);
    
    QRGenerator qr_gen;
    std::string response;
    
    try {
        std::lock_guard<std::mutex> lock(qr_mutex);
        
        if (request.substr(0, 4) == "TEXT") {
            std::string text = request.substr(5);
            qr_gen.generateQR(text);
            response = "QRCODE:" + qr_gen.getQRImage();
        } 
        else if (request.find("GEO:") == 0) {
            size_t comma_pos = request.find(',', 4);
            if (comma_pos == std::string::npos) {
                LOG_ERROR("Invalid GEO format in request: " + request);
                throw std::runtime_error("Invalid GEO format");
            }
            
            std::string lat_str = request.substr(4, comma_pos - 4);
            std::string lon_str = request.substr(comma_pos + 1);
            
            LOG_DEBUG("Parsing coordinates: lat_str=" + lat_str + " lon_str=" + lon_str);
            
            double lat = std::stod(lat_str);
            double lon = std::stod(lon_str);
            
            LOG_DEBUG("Parsed coordinates: lat=" + std::to_string(lat) + 
                     " lon=" + std::to_string(lon));
            
            qr_gen.generateLocationQR(lat, lon);
            response = "QRCODE:" + qr_gen.getQRImage();
        } 
        else {
            LOG_WARNING("Invalid request format: " + request);
            response = "ERROR:Invalid request format";
        }
    } catch (const std::exception& e) {
        LOG_ERROR("Request processing error: " + std::string(e.what()));
        response = "ERROR:" + std::string(e.what());
    }
    
    send(client_socket, response.c_str(), response.size(), 0);
    close(client_socket);
}

int main() {
    Logger::getInstance().init("server.log");
    LOG_INFO("Starting QR code server on port " + std::to_string(PORT));

    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        LOG_ERROR("Socket creation failed");
        perror("socket failed");
        exit(EXIT_FAILURE);
    }
    
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        LOG_ERROR("Setsockopt failed");
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);
    
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        LOG_ERROR("Bind failed");
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    
    if (listen(server_fd, 3) < 0) {
        LOG_ERROR("Listen failed");
        perror("listen");
        exit(EXIT_FAILURE);
    }
    
    std::cout << "Server started on port " << PORT << std::endl;
    std::vector<std::thread> threads;
    
    while (true) {
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
            LOG_ERROR("Accept failed");
            perror("accept");
            continue;
        }
        
        threads.emplace_back(handle_client, new_socket);
    }
    
    for (auto& t : threads) {
        if (t.joinable()) t.join();
    }
    
    return 0;
}
