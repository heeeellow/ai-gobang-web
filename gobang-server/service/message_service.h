#pragma once
#include "../db/dbconn.h"
#include "../model/message.h"
#include <vector>

class MessageService {
public:
    MessageService(DBConn& db) : db_(db) {}

    // 发送消息
    bool send_message(int room_id, int user_id, const std::string& content) {
        std::string sql = "INSERT INTO room_messages (room_id, user_id, content, sent_at) VALUES (" +
            std::to_string(room_id) + "," + std::to_string(user_id) + ",'" + db_.escape(content) + "',NOW())";
        return db_.execute(sql);
    }

    // 拉取房间全部消息
    std::vector<RoomMessage> list_messages(int room_id) {
        std::vector<RoomMessage> msgs;
        auto rows = db_.query("SELECT id,room_id,user_id,content,sent_at FROM room_messages WHERE room_id=" +
                              std::to_string(room_id) + " ORDER BY sent_at ASC");
        for (auto& r : rows)
            msgs.emplace_back(std::stoi(r[0]), std::stoi(r[1]), std::stoi(r[2]), r[3], r[4]);
        return msgs;
    }
private:
    DBConn& db_;
};
