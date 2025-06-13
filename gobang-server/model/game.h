#pragma once
#include <string>

class Game {
public:
    int id;
    int room_id;
    int black_user;
    int white_user;
    int winner;
    std::string reason;
    std::string moves;      // JSON格式
    std::string started_at;
    std::string ended_at;

    Game() = default;
    Game(int id, int room_id, int black_user, int white_user, int winner,
         const std::string& reason, const std::string& moves, const std::string& started_at, const std::string& ended_at)
        : id(id), room_id(room_id), black_user(black_user), white_user(white_user), winner(winner),
          reason(reason), moves(moves), started_at(started_at), ended_at(ended_at) {}
};
