#pragma once
#include <vector>
#include <array>
#include <string>
#include <map>
#include <memory>
#include <functional>
#include "ChessMove.h"
#include "UserConnection.h"
#include "ai_player.h"
#include<mutex>
class GameRoom {
     mutable std::mutex mtx_;
public:
    int id;
    std::string name;
    bool is_ai = false;
    std::string ai_level; // easy/normal/hard
    int ai_user_id = -1;  // 固定AI的user_id/-1
    int room_id;
    int black_id = 0, white_id = 0;
    std::string black_name, white_name;
    int cur_turn_id = 0;
    std::string cur_color = "black";
    std::vector<ChessMove> moves;
    std::array<std::array<int, 15>, 15> board{};
    bool started = false, over = false;
    bool black_ready = false, white_ready = false;
    std::string result_msg;

    std::map<int, std::shared_ptr<UserConnection>> members; // user_id -> conn
std::map<int, bool> ready;  // 新增：记录每个成员的准备状态
    GameRoom(int room_id_);
    
     GameRoom(int id, const std::string& name,
             bool is_ai = false,
             const std::string& ai_level = "easy")
        : id(id), name(name), is_ai(is_ai), ai_level(ai_level) { }

    void add_member(std::shared_ptr<UserConnection> user, bool spectator);
    void remove_member(int user_id);
    void handle_ready(int user_id);
    bool handle_chess_move(int user_id, int x, int y, std::string& win_color);
    bool check_win(int x, int y);
    void reset();
 // 新增AI房自动准备和自动走子接口
    void set_ai_ready();
    bool is_ai_turn() const;
    void ai_move_if_turn(std::function<void(int, int, std::string)> on_ai_move);
};
