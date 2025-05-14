#ifndef LOGGING_H
#define LOGGING_H

#include <string>
#include <fstream>
#include <mutex>
#include <iostream>
#include <iomanip>
#include <ctime>
#include <memory>

class Logger {
public:
    enum LogLevel {
        DEBUG,
        INFO,
        WARNING,
        ERROR
    };

    static Logger& getInstance();
    
    void init(const std::string& filename = "");
    void log(LogLevel level, const std::string& message);

private:
    Logger() = default;
    ~Logger();
    
    std::ofstream file_;
    std::mutex mutex_;
};

#define LOG_DEBUG(msg) Logger::getInstance().log(Logger::DEBUG, msg)
#define LOG_INFO(msg) Logger::getInstance().log(Logger::INFO, msg)
#define LOG_WARNING(msg) Logger::getInstance().log(Logger::WARNING, msg)
#define LOG_ERROR(msg) Logger::getInstance().log(Logger::ERROR, msg)

#endif // LOGGING_H
