#pragma once
#include "db/dbconn.h"
#include "gobang/GameRoomManager.h"
#include "gobang/UserConnection.h"
#include <nlohmann/json.hpp>
#include <unordered_map>
#include <thread>
#include <atomic>
#include <chrono>
class WebSocketServer {
     GameRoomManager& room_mgr;  // 引用
public:
  WebSocketServer(DBConn& db, int port, GameRoomManager& mgr)
        : db_(db), port_(port), room_mgr(mgr) { }
    void start(); // 启动服务
    void stop();  // 停止服务
private:
    int port_;
    DBConn& db_;
    std::atomic<bool> running_;
    std::thread server_thread_;

    std::mutex client_mutex;
    std::unordered_map<int, std::shared_ptr<UserConnection>> fd_to_conn; // fd -> conn

    void server_loop(); // 核心accept/事件循环
    void handle_client(int client_fd);
    void on_message(int fd, const std::string& msg);
    void on_close(int fd);

    void broadcast_room(int room_id, const nlohmann::json& j);
    nlohmann::json build_room_info(const std::shared_ptr<GameRoom>& gr);
};
