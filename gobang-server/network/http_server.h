#pragma once
#include "../db/dbconn.h"
#include "../service/user_service.h"
#include "gobang/GameRoomManager.h"
#include <thread>
#include <nlohmann/json.hpp>

class HttpServer {
 GameRoomManager& room_mgr;  // 引用
public:
    HttpServer(DBConn& db, int port, GameRoomManager& mgr)
        : db_(db), port_(port), room_mgr(mgr) { }

    void start();
    void stop();
private:
    int port_;
    int listenfd_;
    DBConn& db_;
    bool running_;
    std::thread server_thread_;

    void run();
    void handle_connection(int connfd);

    void handle_api(const std::string& method, const std::string& path,
                    const std::string& body, int connfd);

    // 工具
    void send_json(int connfd, const nlohmann::json& res);
};
