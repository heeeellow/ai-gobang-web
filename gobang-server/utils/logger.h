#pragma once
#include <string>
#include <mutex>
#include <iostream>
#include <ctime>

// 线程安全简单日志
class Logger {
public:
    enum Level { INFO, WARNING, ERROR };

    static void log(Level level, const std::string& msg);
    static void info(const std::string& msg) { log(INFO, msg); }
    static void warn(const std::string& msg) { log(WARNING, msg); }
    static void error(const std::string& msg) { log(ERROR, msg); }
};
