#include "GameRoomManager.h"
#include <iostream>

std::shared_ptr<GameRoom> GameRoomManager::get_or_create(int room_id) {
    std::lock_guard<std::mutex> lock(mutex);
    if (rooms.count(room_id) == 0) {
        rooms[room_id] = std::make_shared<GameRoom>(room_id); // 只有普通房能走这里
    }
    return rooms[room_id];
}

void GameRoomManager::remove_empty_rooms() {
    std::lock_guard<std::mutex> lock(mutex);
    for (auto it = rooms.begin(); it != rooms.end();) {
        if (it->second->members.empty()) it = rooms.erase(it);
        else ++it;
    }
}

std::shared_ptr<GameRoom> GameRoomManager::create_ai_room(int user_id, const std::string& username, const std::string& level) {
    int rid = (int)time(nullptr) * 10 + user_id;
    auto room = std::make_shared<GameRoom>(rid, "AI对战", true, level);
    room->black_id = user_id;
    room->black_name = username;
    room->members[user_id] = nullptr; // 玩家

    // 核心：AI方初始化
    room->white_id = -1;
    room->white_name = "AI";
    room->ready[user_id] = false;
    room->ready[-1] = false;
    room->black_ready = false;
    room->white_ready = false;
    room->started = false;
    room->over = false;
    room->cur_turn_id = user_id;;
    room->cur_color = "black";
    room->result_msg.clear();

    rooms[rid] = room;
    return room;
}

void GameRoomManager::remove_user_from_all_rooms(int user_id) {
    std::lock_guard<std::mutex> lock(mutex);
    for (auto& kv : rooms) {
        kv.second->remove_member(user_id);
    }
    remove_empty_rooms();
}