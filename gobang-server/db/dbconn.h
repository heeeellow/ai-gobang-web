#pragma once
#include <mysql/mysql.h>
#include <string>
#include <vector>
#include <mutex>
#include <memory>
#include <stdexcept>
#include "../utils/logger.h"

// 数据库单连接封装，支持多线程
class DBConn {
public:
    DBConn(const std::string& host, const std::string& user,
           const std::string& passwd, const std::string& db, unsigned port);
    ~DBConn();

    // 执行无返回结果SQL（如INSERT/UPDATE/DELETE）
    bool execute(const std::string& sql);

    // 查询SQL，返回二维vector（每行是vector<string>）
    std::vector<std::vector<std::string>> query(const std::string& sql);

    // 辅助：转义SQL字符串
    std::string escape(const std::string& s);

    MYSQL* raw() { return conn_; }
private:
    MYSQL* conn_;
    std::mutex mutex_;
    std::string host_, user_, passwd_, db_;
    unsigned     port_;
    bool ensure_alive();
};
