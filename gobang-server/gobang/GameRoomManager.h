#pragma once
#include "GameRoom.h"
#include <unordered_map>
#include <mutex>
#include <memory>

class GameRoomManager {
public:
    std::unordered_map<int, std::shared_ptr<GameRoom>> rooms;
    std::mutex mutex;

    std::shared_ptr<GameRoom> get_or_create(int room_id);
    void remove_empty_rooms();
     // 新增AI房创建方法
    std::shared_ptr<GameRoom> create_ai_room(int user_id, const std::string& username, const std::string& level);
void remove_user_from_all_rooms(int user_id);
};
