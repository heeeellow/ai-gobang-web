#pragma once
#include <string>

class RoomMember {
public:
    int id;
    int room_id;
    int user_id;
    std::string role;     // black/white/spectator
    bool ready;
    std::string joined_at;

    RoomMember() = default;
    RoomMember(int id, int room_id, int user_id, const std::string& role, bool ready, const std::string& joined_at)
        : id(id), room_id(room_id), user_id(user_id), role(role), ready(ready), joined_at(joined_at) {}
};
