#include "websocket_server.h"
#include "ws_util.h"
#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>


void WebSocketServer::start() {
    running_ = true;
    server_loop(); 
}

void WebSocketServer::stop() {
    running_ = false;
    if (server_thread_.joinable())
        server_thread_.join();
}
void WebSocketServer::server_loop() {
     try {
    int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd < 0) {
    std::cerr << "[WS] socket() failed!\n";
    return;
}
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port_);
    bind(listen_fd, (sockaddr*)&addr, sizeof(addr));
    //listen(listen_fd, 32);
if (listen(listen_fd, 32) < 0) {
    std::cerr << "[WS] listen() failed!\n";
    close(listen_fd);
    return;
}
    Logger::info("WebSocket Listening on port: "+std::to_string(port_));
    while (running_) {
        int client_fd = accept(listen_fd, nullptr, nullptr);
        if (client_fd < 0) continue;
        std::thread(&WebSocketServer::handle_client, this, client_fd).detach();
    }
    close(listen_fd);
    } catch (const std::exception& e) {
        std::cerr << "[WS] Exception: " << e.what() << std::endl;
    } catch (...) {
        std::cerr << "[WS] Unknown Exception!" << std::endl;
    }
}

void WebSocketServer::handle_client(int client_fd) {
    try {
    if (!websocket_handshake(client_fd)) {
        close(client_fd); return;
    }
    {
        std::lock_guard<std::mutex> lock(client_mutex);
        fd_to_conn[client_fd] = std::make_shared<UserConnection>(client_fd);
    }
    std::string msg;
    while (ws_recv_text(client_fd, msg)) {
        on_message(client_fd, msg);
    }
    on_close(client_fd);
    close(client_fd);
     } catch (const std::exception& e) {
        std::cerr << "[WS] Exception in handle_client: " << e.what() << std::endl;
    }
}

void WebSocketServer::on_message(int fd, const std::string& text) {
    if (text.empty()) return;                   // 忽略空帧
    nlohmann::json j;
    try { j = nlohmann::json::parse(text); }
    catch (const std::exception& e) {
        Logger::warn(std::string("Bad JSON:")+e.what());
        return;
    }

    std::string type = j.value("type", "");

   if (type == "join_room") {
    int user_id = j.value("user_id", 0);
    std::string username = j.value("username", "");
    int room_id = j.value("roomId", 0);
    bool spectate = j.value("spectate", false);

    auto conn = fd_to_conn[fd];
    conn->user_id = user_id;
    conn->room_id = room_id;
    conn->username = username;
    conn->spectator = spectate;

    auto gr = room_mgr.get_or_create(room_id);
    gr->add_member(conn, spectate);
    broadcast_room(room_id, build_room_info(gr));
}


    else if (type == "ready") {
        int room_id = fd_to_conn[fd]->room_id;
        int user_id = fd_to_conn[fd]->user_id;
        auto gr = room_mgr.get_or_create(room_id);
        gr->handle_ready(user_id);

        // AI房: 玩家ready时，AI自动准备
        if (gr->is_ai) {
            gr->set_ai_ready();
        }
        broadcast_room(room_id, build_room_info(gr));

        // AI房: 玩家和AI都ready，直接开始
        if (gr->is_ai && gr->black_ready && gr->white_ready && !gr->started) {
            gr->started = true;
            gr->over = false;
            gr->cur_turn_id = gr->black_id;  // 玩家先手
            gr->cur_color = "black";
            broadcast_room(room_id, build_room_info(gr));
            // 如果AI先手，这里可换为gr->cur_turn_id=-1
            // AI走棋逻辑在玩家落子后处理
        }
    }

else if (type == "chess_move") {
    int room_id = fd_to_conn[fd]->room_id;
    int user_id = fd_to_conn[fd]->user_id;
    int x = j.value("x", -1), y = j.value("y", -1);
    auto gr = room_mgr.get_or_create(room_id);
    std::string win_color;
    std::string move_color = gr->cur_color;
    if (gr->handle_chess_move(user_id, x, y, win_color)) {
        nlohmann::json move = {
            {"type","chess_move"},
            {"x",x},{"y",y},{"color",move_color},
            {"nextColor",gr->cur_color},
            {"nextUser",gr->cur_turn_id==gr->black_id?gr->black_name:gr->white_name}
        };
        broadcast_room(room_id, move);
        std::string winner_name;
        if (!win_color.empty()) {
        if (win_color == "black") winner_name = gr->black_name;
    else if (win_color == "white") winner_name = gr->white_name;
    else winner_name = "未知玩家";
    gr->over = true;
    gr->result_msg = winner_name + " 胜利";
    nlohmann::json over = {
        {"type", "game_over"},
        {"resultMsg", gr->result_msg}
    };
    broadcast_room(room_id, over);
    return;
        }

        // --- AI房: 轮到AI时自动走棋 ---
        if (gr->is_ai && gr->is_ai_turn() && !gr->over) {
            std::this_thread::sleep_for(std::chrono::milliseconds(350));
            std::string ai_move_color = gr->cur_color;
            gr->ai_move_if_turn([&](int ai_x, int ai_y, std::string ai_win){
                nlohmann::json ai_move = {
                    {"type","chess_move"},
                    {"x",ai_x},{"y",ai_y},{"color",ai_move_color},
                    {"nextColor",gr->cur_color},
                    {"nextUser",gr->cur_turn_id==gr->black_id?gr->black_name:gr->white_name}
                };
                broadcast_room(room_id, ai_move);
               if (!ai_win.empty()) {
                gr->over = true;
                std::string winner_name;
                if (ai_win == "black") winner_name = gr->black_name;
                else if (ai_win == "white") winner_name = gr->white_name;
                else winner_name = "未知玩家";
                gr->result_msg = winner_name + " 胜利";
                nlohmann::json over = {
                    {"type", "game_over"},
                    {"resultMsg", gr->result_msg}
                };
                broadcast_room(room_id, over);
            }

            });
        }
    }
}


    else if (type == "chat") {
        int room_id = fd_to_conn[fd]->room_id;
        std::string text = j.value("text", "");
        std::string username = fd_to_conn[fd]->username;
     auto t = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
     std::stringstream ss;
     ss << std::put_time(std::localtime(&t), "%H:%M:%S");
     std::string str_time = ss.str();
        nlohmann::json chat = {
            {"type","chat"},
            {"user",username},
            {"text",text},
            {"time",str_time}
        };
        broadcast_room(room_id, chat);
    }

    else if (type == "giveup") {
        int room_id = fd_to_conn[fd]->room_id;
        int user_id = fd_to_conn[fd]->user_id;
        auto gr = room_mgr.get_or_create(room_id);
        gr->over = true;
        gr->result_msg = fd_to_conn[fd]->username+"认输";
        nlohmann::json over = {
            {"type", "game_over"},
            {"resultMsg", gr->result_msg}
        };
        broadcast_room(room_id, over);
    }

else if (type == "leave_room") {
    int user_id = fd_to_conn[fd]->user_id;
    // 删除所有房间的该用户
    for (auto& kv : room_mgr.rooms) {
        auto& gr = kv.second;
        if (gr->members.count(user_id)) {
            gr->remove_member(user_id);
            broadcast_room(gr->id, build_room_info(gr));
        }
    }
    room_mgr.remove_empty_rooms();
    fd_to_conn[fd]->room_id = 0;
    fd_to_conn[fd]->user_id = 0;
}


    // ...其他消息类型...
}

void WebSocketServer::on_close(int fd) {
    std::lock_guard<std::mutex> lock(client_mutex);
    if (!fd_to_conn.count(fd)) return;
    int room_id = fd_to_conn[fd]->room_id;
    int user_id = fd_to_conn[fd]->user_id;
    if (room_id) {
        auto gr = room_mgr.get_or_create(room_id);
        gr->remove_member(user_id);
        broadcast_room(room_id, build_room_info(gr));
        room_mgr.remove_empty_rooms();
    }
    fd_to_conn.erase(fd);
}
void WebSocketServer::broadcast_room(int room_id, const nlohmann::json& j) {
    auto gr = room_mgr.get_or_create(room_id);
    std::string s = j.dump();
    for (auto& kv : gr->members) {
        kv.second->send_text(s);
    }
}
nlohmann::json WebSocketServer::build_room_info(const std::shared_ptr<GameRoom>& gr) {
    nlohmann::json j;
    j["type"] = "room_info";
    j["black"] = gr->black_name.empty() ? "黑方" : gr->black_name;
    j["white"] = gr->white_name.empty() ? (gr->is_ai ? "AI" : "白方") : gr->white_name;
    // currentUser 用 cur_turn_id 匹配用户名
    std::string turn_name;
    if (gr->cur_turn_id == gr->black_id) turn_name = gr->black_name;
    else if (gr->cur_turn_id == gr->white_id) turn_name = gr->white_name;
    else if (gr->cur_turn_id == -1) turn_name = "AI";
    j["currentUser"] = turn_name;
    j["currentColor"] = gr->cur_color;
    nlohmann::json moves = nlohmann::json::array();
    for (const auto& m : gr->moves)
        moves.push_back({{"x", m.x}, {"y", m.y}, {"color", m.color}});
    j["board"] = moves;
    j["started"] = gr->started;
    j["over"] = gr->over;
    j["resultMsg"] = gr->result_msg;
    return j;
}
