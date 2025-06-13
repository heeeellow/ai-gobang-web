#include "GameRoom.h"
#include <iostream>
#include <mutex>
GameRoom::GameRoom(int room_id_) : room_id(room_id_) {
    board.fill({});
}

void GameRoom::add_member(std::shared_ptr<UserConnection> user, bool spectator) {
    std::lock_guard<std::mutex> lock(mtx_);
    if (!user) return;
    members[user->user_id] = user;
    if (!spectator) {
        if (!is_ai) {
            if (black_id == 0) { black_id = user->user_id; black_name = user->username; }
            else if (white_id == 0 && user->user_id != black_id) { white_id = user->user_id; white_name = user->username; }
        } // AI房白方始终为AI
    }
}


void GameRoom::remove_member(int user_id) {
    std::lock_guard<std::mutex> lock(mtx_);
    members.erase(user_id);
    if (user_id == black_id) { black_id = 0; black_name.clear(); black_ready = false; }
    if (user_id == white_id) { white_id = 0; white_name.clear(); white_ready = false; }
}

void GameRoom::handle_ready(int user_id) {
    if (user_id == black_id) black_ready = true;
    if (user_id == white_id) white_ready = true;
    if (black_ready && white_ready && !started) {
        started = true;
        over = false;
        cur_turn_id = black_id;
        cur_color = "black";
        moves.clear();
        board.fill({});
        result_msg.clear();
    }
}
bool GameRoom::handle_chess_move(int user_id, int x, int y, std::string& win_color) {
    std::lock_guard<std::mutex> lock(mtx_);
    if (!started || over) return false;
    if (user_id != cur_turn_id) return false;
    if (x < 0 || x >= 15 || y < 0 || y >= 15 || board[y][x] != 0) return false;
    std::string color = cur_color; // 下这一步的颜色
    moves.push_back({x, y, color});
    board[y][x] = (color == "black" ? 1 : 2);

    // 判断是否胜利
    if (check_win(x, y)) {
        over = true;
        win_color = color;
        if (color == "black") result_msg = "黑方(" + black_name + ")胜利";
        else result_msg = "白方(" + white_name + ")胜利";
    } else {
        win_color.clear();
        // 切换回合
        if (cur_color == "black") { cur_color = "white"; cur_turn_id = white_id; }
        else { cur_color = "black"; cur_turn_id = black_id; }
    }
    return true;
}

bool GameRoom::check_win(int x, int y) {
    int color = board[y][x];
    const int dirs[4][2] = {{1,0},{0,1},{1,1},{1,-1}};
    for (auto& d : dirs) {
        int cnt = 1;
        for (int i=1;i<5;i++) {
            int nx = x+i*d[0], ny = y+i*d[1];
            if (nx<0||nx>=15||ny<0||ny>=15) break;
            if (board[ny][nx]!=color) break;
            cnt++;
        }
        for (int i=1;i<5;i++) {
            int nx = x-i*d[0], ny = y-i*d[1];
            if (nx<0||nx>=15||ny<0||ny>=15) break;
            if (board[ny][nx]!=color) break;
            cnt++;
        }
        if (cnt>=5) return true;
    }
    return false;
}
void GameRoom::reset() {
    black_ready = white_ready = false;
    started = over = false;
    moves.clear();
    board.fill({});
    result_msg.clear();
}


void GameRoom::set_ai_ready() {
    ready[-1] = true;
    white_ready = true;
}


bool GameRoom::is_ai_turn() const {
    return is_ai && started && !over && cur_turn_id == -1; // AI始终-1
}

// 封装AI自动走子（提供回调，便于WS广播）
void GameRoom::ai_move_if_turn(std::function<void(int, int, std::string)> on_ai_move) {
    if (!is_ai_turn()) return;
    // 调用AI极大极小算法
    int ai_x = -1, ai_y = -1;
    get_ai_move(board, ai_level, ai_x, ai_y); // ai_player.h/cpp 提供
    std::string win_color;
    bool ok = handle_chess_move(-1, ai_x, ai_y, win_color); // AI user_id = -1
    if (on_ai_move && ok) {
        on_ai_move(ai_x, ai_y, win_color);
    }
}