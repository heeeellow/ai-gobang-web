#pragma once
#include "../db/dbconn.h"
#include "../model/game.h"
#include <optional>
#include <vector>

class GameService {
public:
    GameService(DBConn& db) : db_(db) {}

    // 新建一场对局
    std::optional<int> create_game(int room_id, int black_user, int white_user) {
        std::string sql = "INSERT INTO games (room_id, black_user, white_user, moves, started_at) VALUES (" +
            std::to_string(room_id) + "," + std::to_string(black_user) + "," + std::to_string(white_user) +
            ",'[]',NOW())";
        if (!db_.execute(sql)) return std::nullopt;
        auto rows = db_.query("SELECT LAST_INSERT_ID()");
        if (rows.empty()) return std::nullopt;
return std::stoi(rows[0][0]);

    }

    // 更新棋谱和结果
    void finish_game(int game_id, int winner, const std::string& reason, const std::string& moves) {
        std::string sql = "UPDATE games SET winner=" + std::to_string(winner) +
                          ", reason='" + db_.escape(reason) + "', moves='" + db_.escape(moves) +
                          "', ended_at=NOW() WHERE id=" + std::to_string(game_id);
        db_.execute(sql);
    }

    // 获取对局
    std::optional<Game> get_game(int game_id) {
        auto rows = db_.query("SELECT id,room_id,black_user,white_user,winner,reason,moves,started_at,ended_at FROM games WHERE id=" +
                              std::to_string(game_id));
        if (rows.empty()) return std::nullopt;
        auto& r = rows[0];
        return Game(std::stoi(r[0]), std::stoi(r[1]), std::stoi(r[2]), std::stoi(r[3]), std::stoi(r[4]),
                    r[5], r[6], r[7], r[8]);
    }

    // 获取房间历史对局
    std::vector<Game> list_games_by_room(int room_id) {
        std::vector<Game> games;
        auto rows = db_.query("SELECT id,room_id,black_user,white_user,winner,reason,moves,started_at,ended_at FROM games WHERE room_id=" +
                              std::to_string(room_id));
        for (auto& r : rows)
            games.emplace_back(std::stoi(r[0]), std::stoi(r[1]), std::stoi(r[2]), std::stoi(r[3]), std::stoi(r[4]),
                              r[5], r[6], r[7], r[8]);
        return games;
    }
private:
    DBConn& db_;
};
