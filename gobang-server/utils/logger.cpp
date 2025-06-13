#include "logger.h"

void Logger::log(Level level, const std::string& msg) {
    static std::mutex mtx;
    std::lock_guard<std::mutex> lock(mtx);
    const char* level_str[] = {"[INFO]", "[WARN]", "[ERROR]"};
    std::time_t now = std::time(nullptr);
    char buf[32];
    std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", std::localtime(&now));
    std::cout << buf << " " << level_str[level] << " " << msg << std::endl;
}
