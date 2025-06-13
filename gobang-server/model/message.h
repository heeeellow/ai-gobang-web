#pragma once
#include <string>

class RoomMessage {
public:
    int id;
    int room_id;
    int user_id;
    std::string content;
    std::string sent_at;

    RoomMessage() = default;
    RoomMessage(int id, int room_id, int user_id, const std::string& content, const std::string& sent_at)
        : id(id), room_id(room_id), user_id(user_id), content(content), sent_at(sent_at) {}
};
