#pragma once
#include <string>
#include <mutex>
#include <iostream>
#include <ctime>

// 线程安全简单日志
class Logger {
public:
    enum Level { INFO, WARNING, ERROR };

    static void log(Level level, const std::string& msg) {
    static std::mutex mtx;
    std::lock_guard<std::mutex> lock(mtx);
    const char* level_str[] = {"[INFO]", "[WARN]", "[ERROR]"};
    std::time_t now = std::time(nullptr);
    char buf[32];
    std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", std::localtime(&now));
    std::cout << buf << " " << level_str[level] << " " << msg << std::endl;
}

    static void info(const std::string& msg) { log(INFO, msg); }
    static void warn(const std::string& msg) { log(WARNING, msg); }
    static void error(const std::string& msg) { log(ERROR, msg); }
};
