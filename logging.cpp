#include "logging.h"

Logger& Logger::getInstance() {
    static Logger instance;
    return instance;
}

void Logger::init(const std::string& filename) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (filename.empty()) {
        file_.copyfmt(std::cout);
        file_.clear(std::cout.rdstate());
        file_.basic_ios<char>::rdbuf(std::cout.rdbuf());
    } else {
        file_.open(filename, std::ios::out | std::ios::app);
        if (!file_.is_open()) {
            throw std::runtime_error("Failed to open log file");
        }
    }
}

void Logger::log(LogLevel level, const std::string& message) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto now = std::time(nullptr);
    auto tm = *std::localtime(&now);

    file_ << std::put_time(&tm, "%Y-%m-%d %H:%M:%S") << " [";
    switch(level) {
        case DEBUG: file_ << "DEBUG"; break;
        case INFO: file_ << "INFO"; break;
        case WARNING: file_ << "WARNING"; break;
        case ERROR: file_ << "ERROR"; break;
    }
    file_ << "] " << message << std::endl;
}

Logger::~Logger() {
    if (file_.is_open()) {
        file_.close();
    }
}
