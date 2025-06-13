#pragma once
#include <string>

class Room {
public:
    int id;
    std::string name;
    std::string password_hash; // 有则为房间密码的SHA256
    std::string status;        // waiting/playing/full
    int created_by;            // 创建者用户ID
    std::string created_at;
    std::string updated_at;

    Room() = default;
    Room(int id, const std::string& name, const std::string& password_hash,
         const std::string& status, int created_by, const std::string& created_at, const std::string& updated_at)
        : id(id), name(name), password_hash(password_hash), status(status), created_by(created_by),
          created_at(created_at), updated_at(updated_at) {}
};
