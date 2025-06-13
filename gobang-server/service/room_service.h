#pragma once
#include "../db/dbconn.h"
#include "../model/room.h"
#include "../utils/password_hash.h"
#include <vector>
#include <optional>

class RoomService {
public:
    RoomService(DBConn& db) : db_(db) {}

    // 创建房间，返回房间id
    std::optional<int> create_room(const std::string& name, const std::string& password, int user_id) {
        std::string pw_hash = password.empty() ? "" : sha256(password);
        std::string sql = "INSERT INTO rooms (name, password, status, created_by, created_at, updated_at) VALUES ('"
            + db_.escape(name) + "','" + pw_hash + "','waiting'," + std::to_string(user_id) + ",NOW(),NOW())";
        if (!db_.execute(sql)) return std::nullopt;
        auto rows = db_.query("SELECT LAST_INSERT_ID()");
        if (rows.empty()) return std::nullopt;
return std::stoi(rows[0][0]);

    }

    // 获取房间信息
    std::optional<Room> get_room(int id) {
        auto rows = db_.query("SELECT id,name,password,status,created_by,created_at,updated_at FROM rooms WHERE id=" + std::to_string(id));
        if (rows.empty()) return std::nullopt;
        auto& r = rows[0];
        return Room(
            std::stoi(r[0]), r[1], r[2], r[3],
            std::stoi(r[4]), r[5], r[6]
        );
    }

    // 获取所有房间（可以加筛选条件）
    std::vector<Room> list_rooms() {
        std::vector<Room> rooms;
        auto rows = db_.query("SELECT id,name,password,status,created_by,created_at,updated_at FROM rooms WHERE status != 'full'");
        for (auto& r : rows)
            rooms.emplace_back(std::stoi(r[0]), r[1], r[2], r[3], std::stoi(r[4]), r[5], r[6]);
        return rooms;
    }

    // 校验房间密码
    bool check_room_password(int room_id, const std::string& input_password) {
        auto rows = db_.query("SELECT password FROM rooms WHERE id=" + std::to_string(room_id));
        if (rows.empty()) return false;
        std::string stored_hash = rows[0][0];
        if (stored_hash.empty()) return input_password.empty();
        return stored_hash == sha256(input_password);
    }

    // 设置房间状态
    void set_room_status(int room_id, const std::string& status) {
        db_.execute("UPDATE rooms SET status='" + db_.escape(status) + "', updated_at=NOW() WHERE id=" + std::to_string(room_id));
    }

    // 删除房间
    void delete_room(int room_id) {
        db_.execute("DELETE FROM rooms WHERE id=" + std::to_string(room_id));
    }



private:
    DBConn& db_;
};
