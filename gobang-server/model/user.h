#pragma once
#include <string>

class User {
public:
    int id;
    std::string username;
    std::string password_hash;
    std::string email;
    std::string token;
    bool is_online;
    std::string last_login;
    std::string created_at;

    User() = default;
    User(int id, const std::string& username, const std::string& password_hash,
         const std::string& email, const std::string& token, bool is_online,
         const std::string& last_login, const std::string& created_at)
        : id(id), username(username), password_hash(password_hash),
          email(email), token(token), is_online(is_online),
          last_login(last_login), created_at(created_at) {}
};
