#include "dbconn.h"
#include <iostream>

DBConn::DBConn(const std::string& host, const std::string& user,
               const std::string& passwd, const std::string& db, unsigned port) 
               : host_(host), user_(user), passwd_(passwd), db_(db), port_(port)
        {
    conn_ = mysql_init(nullptr);
    if (!mysql_real_connect(conn_, host.c_str(), user.c_str(), passwd.c_str(), db.c_str(), port, nullptr, 0)) {
        Logger::error(mysql_error(conn_));
        throw std::runtime_error(mysql_error(conn_));
    }
    mysql_set_character_set(conn_, "utf8mb4");
    Logger::info("MySQL Connected.");
}

DBConn::~DBConn() {
    if (conn_) {
        mysql_close(conn_);
        Logger::info("MySQL Disconnected.");
    }
}


bool DBConn::ensure_alive() {
    if (mysql_ping(conn_) == 0) return true;          // OK
    Logger::warn("MySQL reconnecting…");
    MYSQL* new_conn = mysql_init(nullptr);
    if (!mysql_real_connect(new_conn,host_.c_str(),user_.c_str(),
            passwd_.c_str(),db_.c_str(),port_,nullptr,0)) {
       Logger::error(mysql_error(new_conn));
        return false;
    }
   mysql_set_character_set(new_conn,"utf8mb4");
    std::swap(conn_, new_conn);               // 热切换
    if (new_conn) mysql_close(new_conn);
    return true;
}

bool DBConn::execute(const std::string& sql) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!ensure_alive()) return false;
    if (mysql_query(conn_, sql.c_str())) {
        Logger::error("SQL Execute Error: " + std::string(mysql_error(conn_)) + " | SQL: " + sql);
        return false;
    }
    return true;
}

std::vector<std::vector<std::string>> DBConn::query(const std::string& sql) {
    std::vector<std::vector<std::string>> result;
    std::lock_guard<std::mutex> lock(mutex_);
    if (!ensure_alive()) return result;
    if (mysql_query(conn_, sql.c_str())) {
        Logger::error("SQL Query Error: " + std::string(mysql_error(conn_)) + " | SQL: " + sql);
        return result;
    }
    MYSQL_RES* res = mysql_store_result(conn_);
    if (!res) return result;
    MYSQL_ROW row;
    unsigned num = mysql_num_fields(res);
    while ((row = mysql_fetch_row(res))) {
        std::vector<std::string> r;
        for (unsigned i = 0; i < num; ++i)
            r.push_back(row[i] ? row[i] : "");
        result.push_back(std::move(r));
    }
    mysql_free_result(res);
    return result;
}

std::string DBConn::escape(const std::string& s) {
    char* buf = new char[s.size()*2+1];
    mysql_real_escape_string(conn_, buf, s.c_str(), s.size());
    std::string r = buf;
    delete[] buf;
    return r;
}
