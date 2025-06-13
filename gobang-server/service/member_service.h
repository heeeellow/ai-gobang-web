#pragma once
#include "../db/dbconn.h"
#include "../model/member.h"
#include <vector>
#include <optional>

class RoomMemberService {
public:
    RoomMemberService(DBConn& db) : db_(db) {}

    // 加入房间
    bool join_room(int room_id, int user_id, const std::string& role) {
        std::string sql = "INSERT INTO room_members (room_id, user_id, role, ready, joined_at) VALUES (" +
            std::to_string(room_id) + "," + std::to_string(user_id) + ",'" + db_.escape(role) + "',0,NOW())";
        return db_.execute(sql);
    }

    // 离开房间
    void leave_room(int room_id, int user_id) {
        db_.execute("DELETE FROM room_members WHERE room_id=" + std::to_string(room_id) + " AND user_id=" + std::to_string(user_id));
    }

    // 获取房间成员
    std::vector<RoomMember> list_members(int room_id) {
        std::vector<RoomMember> members;
        auto rows = db_.query("SELECT id,room_id,user_id,role,ready,joined_at FROM room_members WHERE room_id=" + std::to_string(room_id));
        for (auto& r : rows)
            members.emplace_back(std::stoi(r[0]), std::stoi(r[1]), std::stoi(r[2]), r[3], r[4] == "1", r[5]);
        return members;
    }

    // 设置成员准备状态
    void set_ready(int room_id, int user_id, bool ready) {
        db_.execute("UPDATE room_members SET ready=" + std::to_string(ready ? 1 : 0) +
                    " WHERE room_id=" + std::to_string(room_id) + " AND user_id=" + std::to_string(user_id));
    }

    // 设置成员身份
    void set_role(int room_id, int user_id, const std::string& role) {
        db_.execute("UPDATE room_members SET role='" + db_.escape(role) + "' WHERE room_id=" +
                    std::to_string(room_id) + " AND user_id=" + std::to_string(user_id));
    }

    // 获取某成员
    std::optional<RoomMember> get_member(int room_id, int user_id) {
        auto rows = db_.query("SELECT id,room_id,user_id,role,ready,joined_at FROM room_members WHERE room_id=" +
                              std::to_string(room_id) + " AND user_id=" + std::to_string(user_id));
        if (rows.empty()) return std::nullopt;
        auto& r = rows[0];
        return RoomMember(std::stoi(r[0]), std::stoi(r[1]), std::stoi(r[2]), r[3], r[4] == "1", r[5]);
    }
        // 在 RoomMemberService 中添加
int count_players(int room_id) {
    // 只统计黑白方（观战不算）
    auto rows = db_.query("SELECT COUNT(*) FROM room_members WHERE room_id=" + std::to_string(room_id) +
                          " AND (role='black' OR role='white')");
    if (!rows.empty()) return std::stoi(rows[0][0]);
    return 0;
}

    // 删除房间所有成员
    void remove_all(int room_id) {
        db_.execute("DELETE FROM room_members WHERE room_id=" + std::to_string(room_id));
    }
private:
    DBConn& db_;
};
