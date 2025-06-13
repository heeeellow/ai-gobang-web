#pragma once
#include "../db/dbconn.h"
#include "../model/user.h"
#include "../utils/password_hash.h"
#include "../utils/token.h"
#include <optional>

class UserService {
public:
    UserService(DBConn& db) : db_(db) {}

    // 注册用户
    bool register_user(const std::string& username, const std::string& password, const std::string& email, std::string& err) {
        // 检查是否重名
        auto exists = db_.query("SELECT id FROM users WHERE username='" + db_.escape(username) + "'");
        if (!exists.empty()) {
            err = "用户名已存在";
            return false;
        }
        std::string pw_hash = sha256(password);
        std::string sql = "INSERT INTO users (username, password, email, created_at) VALUES ('" +
                   db_.escape(username) + "','" + pw_hash + "','" + db_.escape(email) + "',NOW())";
        if (!db_.execute(sql)) {
            err = "数据库错误";
            return false;
        }
        return true;
    }

    // 登录，返回token
    std::optional<std::string> login(const std::string& username, const std::string& password, std::string& err) {
        auto rows = db_.query("SELECT id, password FROM users WHERE username='" + db_.escape(username) + "'");
        if (rows.empty()) {
            err = "用户名不存在";
            return std::nullopt;
        }
        std::string pw_hash = sha256(password);
        if (rows[0][1] != pw_hash) {
            err = "密码错误";
            return std::nullopt;
        }
        std::string new_token = gen_token();
        // 先将所有同名用户token置空（本项目唯一用户名即可）
    db_.execute("UPDATE users SET is_online=0, token=NULL WHERE username='" + db_.escape(username) + "'");
    // 给本次登录赋新token和在线状态
    db_.execute("UPDATE users SET is_online=1, token='" + new_token +
                "', last_login=NOW() WHERE username='" + db_.escape(username) + "'");
        return new_token;
    }

    // 通过token获取用户信息
    std::optional<User> get_user_by_token(const std::string& token) {
        auto rows = db_.query("SELECT id,username,password,email,token,is_online,last_login,created_at FROM users WHERE token='" + db_.escape(token) + "'");
        if (rows.empty()) return std::nullopt;
        const auto& r = rows[0];
        return User(
            std::stoi(r[0]), r[1], r[2], r[3], r[4],
            r[5] == "1", r[6], r[7]
        );
    }

    // 通过用户名查找用户
    std::optional<User> get_user_by_username(const std::string& username) {
        auto rows = db_.query("SELECT id,username,password,email,token,is_online,last_login,created_at FROM users WHERE username='" + db_.escape(username) + "'");
        if (rows.empty()) return std::nullopt;
        const auto& r = rows[0];
        return User(
            std::stoi(r[0]), r[1], r[2], r[3], r[4],
            r[5] == "1", r[6], r[7]
        );
    }

    // 用户登出
    void logout(const std::string& username) {
        db_.execute("UPDATE users SET is_online=0, token='' WHERE username='" + db_.escape(username) + "'");
    }

private:
    DBConn& db_;
};
