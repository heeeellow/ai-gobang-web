#include "db/dbconn.h"
#include "network/http_server.h"
#include "network/websocket_server.h"
#include <csignal>
#include <thread>
#include "gobang/GameRoomManager.h"
HttpServer* global_http = nullptr;
WebSocketServer* global_ws = nullptr;
 GameRoomManager room_mgr;   // 全局唯一对象
void sigint_handler(int) {
    if (global_http) global_http->stop();
    if (global_ws) global_ws->stop();
    exit(0);
}

int main() {

 DBConn db("1.95.61.131", "root", "Yy2661320808@!", "GoBang", 3306);
    HttpServer http(db, 5555, room_mgr);
    WebSocketServer ws(db, 5566, room_mgr);
    global_http = &http;
    global_ws = &ws;
    std::thread http_thread([&http]{ http.start(); });
    std::thread ws_thread([&ws] { ws.start(); });
    std::signal(SIGINT, sigint_handler);
    http_thread.join();
    ws_thread.join();
    return 0;
}
