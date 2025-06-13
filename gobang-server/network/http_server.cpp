#include "http_server.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sstream>
#include <iostream>
#include "../utils/logger.h"
#include "../service/room_service.h"
#include "../service/member_service.h"
#include "../service/user_service.h"
#include "../service/message_service.h"
#include <nlohmann/json.hpp>



std::string get_username_by_id(DBConn& db, int user_id) {
    std::string username = "玩家";
    auto rows = db.query("SELECT username FROM users WHERE id=" + std::to_string(user_id));
    if (!rows.empty() && !rows[0].empty()) {
        username = rows[0][0];
    }
    return username;
}



void HttpServer::start() {
   running_ = true;
 
    run(); // 只要阻塞监听即可
   
}

void HttpServer::stop() {
    running_ = false;
    if (listenfd_ > 0) close(listenfd_);
    if (server_thread_.joinable()) server_thread_.join();
}

void HttpServer::run() {
    listenfd_ = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in addr{};
    addr.sin_family = AF_INET; addr.sin_addr.s_addr = INADDR_ANY; addr.sin_port = htons(port_);
    bind(listenfd_, (sockaddr*)&addr, sizeof(addr));
    listen(listenfd_, 32);
   Logger::info("HTTP服务启动");
    while (running_) {
        int connfd = accept(listenfd_, nullptr, nullptr);
        if (connfd < 0) continue;
        std::thread(&HttpServer::handle_connection, this, connfd).detach();
    }
}

void HttpServer::handle_connection(int connfd) {
    char buf[8192];
    int n = read(connfd, buf, sizeof(buf)-1);
    if (n <= 0) { close(connfd); return; }
    buf[n] = 0;
    std::string req(buf);

    // 解析请求行
    std::istringstream ss(req);
    std::string line;
    std::getline(ss, line);
    std::string method, path, ver;
    std::istringstream ls(line);
    ls >> method >> path >> ver;

    // 预检请求直接回复CORS允许
    if (method == "OPTIONS") {
        std::string res_str =
            "HTTP/1.1 200 OK\r\n"
            "Access-Control-Allow-Origin: *\r\n"
            "Access-Control-Allow-Headers: Content-Type\r\n"
            "Access-Control-Allow-Methods: *\r\n"
            "Content-Length: 0\r\n"
            "\r\n";
        write(connfd, res_str.c_str(), res_str.size());
        close(connfd);
        return;
    }

    // 找到body
    std::string body;
    auto pos = req.find("\r\n\r\n");
    if (pos != std::string::npos) body = req.substr(pos+4);

    // 路由分发
    handle_api(method, path, body, connfd);

    close(connfd);
}


// 路由实现：这里只实现登录、注册，后续逐步完善
void HttpServer::handle_api(const std::string& method, const std::string& path,
                            const std::string& body, int connfd) {
    UserService user_service(db_);
    RoomService room_service(db_);
    RoomMemberService member_service(db_);
    MessageService message_service(db_);
    nlohmann::json res;

    // 1. 登录
    if (method == "POST" && path == "/api/auth/login") {
        nlohmann::json j = nlohmann::json::parse(body);
        std::string err;
        auto token = user_service.login(j["username"], j["password"], err);
        if (token) {
            // 前端建议把 {id, username} 存 localStorage
            auto user = user_service.get_user_by_username(j["username"]);
            res["success"] = true;
            res["token"] = *token;
            if (user) {
                res["user"] = {
                    {"id", user->id},
                    {"username", user->username},
                    {"email", user->email}
                };
            }
        } else {
            res["success"] = false;
            res["msg"] = err;
        }
        send_json(connfd, res);
        return;
    }

    // 2. 注册
    if (method == "POST" && path == "/api/auth/register") {
        nlohmann::json j = nlohmann::json::parse(body);
        std::string err;
        bool ok = user_service.register_user(j["username"], j["password"], j["email"], err);
        if (ok) {
            res["success"] = true;
        } else {
            res["success"] = false;
            res["msg"] = err;
        }
        send_json(connfd, res);
        return;
    }

    // 3. 获取当前登录用户信息（通过token，建议GET /api/auth/me?token=xxx）
    if (method == "GET" && path.find("/api/auth/me") == 0) {
        std::string token;
        // 简单token解析（支持URL参数 ?token=xxx）
        auto qpos = path.find("?");
        if (qpos != std::string::npos) {
            std::string q = path.substr(qpos + 1);
            auto tpos = q.find("token=");
            if (tpos != std::string::npos)
                token = q.substr(tpos + 6);
        }
        if (token.empty()) token = ""; // 防止未传token
        auto user = user_service.get_user_by_token(token);
        if (user) {
            res["success"] = true;
            res["user"] = {
                {"id", user->id},
                {"username", user->username},
                {"email", user->email},
                {"is_online", user->is_online}
            };
        } else {
            res["success"] = false;
            res["msg"] = "无效token";
        }
        send_json(connfd, res);
        return;
    }

    // 4. 获取所有在线用户
    if (method == "GET" && path == "/api/users/online") {
        auto rows = db_.query("SELECT id,username,email FROM users WHERE is_online=1");
        nlohmann::json arr = nlohmann::json::array();
        for (auto& r : rows) {
            arr.push_back({
                {"id", std::stoi(r[0])},
                {"username", r[1]},
                {"email", r[2]}
            });
        }
        res["success"] = true;
        res["users"] = arr;
        send_json(connfd, res);
        return;
    }

    // 5. 房间列表（GET /api/rooms）
if (method == "GET" && path == "/api/rooms") {
    auto rooms = room_service.list_rooms();
    nlohmann::json arr = nlohmann::json::array();
    for (const auto& r : rooms) {
        int player_count = member_service.count_players(r.id);
        // 清理0人房间
        if (player_count < 1) {
            member_service.remove_all(r.id);
            room_service.delete_room(r.id);
            continue;
        }
        // 判定状态
        std::string status = "waiting";
        if (player_count == 2)
            status = "full";
        arr.push_back({
            {"id", r.id},
            {"name", r.name},
            {"status", status},
            {"has_password", !r.password_hash.empty()},
            {"player_count", player_count}
        });
    }
    res["success"] = true;
    res["list"] = arr;
    send_json(connfd, res);
    return;
}


    // 6. 创建房间（POST /api/rooms）
    if (method == "POST" && path == "/api/rooms") {
        nlohmann::json j = nlohmann::json::parse(body);
        std::string name = j["name"];
        std::string password = j.value("password", "");
        int user_id = j["user_id"];
        auto room_id = room_service.create_room(name, password, user_id);
        if (room_id) {
            res["success"] = true;
            res["room_id"] = *room_id;
            member_service.join_room(*room_id, user_id, "black"); // 创建者为黑方
        } else {
            res["success"] = false;
            res["msg"] = "创建房间失败";
        }
        send_json(connfd, res);
        return;
    }

    //ai rooms
   if (method == "POST" && path == "/api/rooms/bot") {
    nlohmann::json j = nlohmann::json::parse(body);
    int user_id = j["user_id"];
    std::string level = j.value("level", "easy");

    // 取用户名
    std::string username = get_username_by_id(db_, user_id);

    // AI房只用内存
    auto gr = room_mgr.create_ai_room(user_id, username, level);

    res["success"] = true;
    res["room_id"] = gr->id;
    send_json(connfd, res);
    return;
}



    // 7. 加入房间（POST /api/rooms/join）
    if (method == "POST" && path == "/api/rooms/join") {
        nlohmann::json j = nlohmann::json::parse(body);
        int room_id = j["room_id"];
        int user_id = j["user_id"];
        std::string password = j.value("password", "");
        if (!room_service.check_room_password(room_id, password)) {
            res["success"] = false;
            res["msg"] = "密码错误";
            send_json(connfd, res);
            return;
        }
        auto members = member_service.list_members(room_id);
        int player_count = 0;
        for (auto& m : members)
            if (m.role == "black" || m.role == "white") player_count++;
        std::string role = "white";
        if (player_count >= 2) {
            role = "spectator";
            
        } else {
            bool has_black = false, has_white = false;
            for (auto& m : members) {
                if (m.role == "black") has_black = true;
                if (m.role == "white") has_white = true;
            }
            if (!has_black) role = "black";
            else if (!has_white) role = "white";
            else role = "spectator";
        }
        member_service.join_room(room_id, user_id, role);
        res["success"] = true;
        res["role"] = role;
        send_json(connfd, res);
        return;
    }

    // 8. 退出房间（POST /api/rooms/leave）
    if (method == "POST" && path == "/api/rooms/leave") {
        nlohmann::json j = nlohmann::json::parse(body);
        int room_id = j["room_id"];
        int user_id = j["user_id"];
        member_service.leave_room(room_id, user_id);
        res["success"] = true;
        send_json(connfd, res);
        return;
    }

    // 9. 房间详情（GET /api/rooms/{id}，且不是messages接口）
    if (method == "GET" && path.find("/api/rooms/") == 0 &&
        path.find("/messages") == std::string::npos) {
        int room_id = std::stoi(path.substr(std::string("/api/rooms/").length()));
        auto room = room_service.get_room(room_id);
        if (!room) {
            res["success"] = false;
            res["msg"] = "房间不存在";
            send_json(connfd, res);
            return;
        }
        auto members = member_service.list_members(room_id);
        nlohmann::json memarr = nlohmann::json::array();
        for (auto& m : members) {
            memarr.push_back({
                {"user_id", m.user_id},
                {"role", m.role},
                {"ready", m.ready}
            });
        }
        res["success"] = true;
        res["room"] = {
            {"id", room->id}, {"name", room->name}, {"status", room->status}, {"created_by", room->created_by}
        };
        res["members"] = memarr;
        send_json(connfd, res);
        return;
    }

    // 10. 设置准备状态（POST /api/rooms/prepare）
    if (method == "POST" && path == "/api/rooms/prepare") {
        nlohmann::json j = nlohmann::json::parse(body);
        int room_id = j["room_id"];
        int user_id = j["user_id"];
        bool ready = j["ready"];
        member_service.set_ready(room_id, user_id, ready);
        res["success"] = true;
        send_json(connfd, res);
        return;
    }

    // 11. 聊天消息发送（POST /api/rooms/chat）
    if (method == "POST" && path == "/api/rooms/chat") {
        nlohmann::json j = nlohmann::json::parse(body);
        int room_id = j["room_id"];
        int user_id = j["user_id"];
        std::string content = j["content"];
        message_service.send_message(room_id, user_id, content);
        res["success"] = true;
        send_json(connfd, res);
        return;
    }

    // 12. 聊天消息列表（GET /api/rooms/{room_id}/messages）
    if (method == "GET" && path.find("/api/rooms/") == 0 &&
        path.find("/messages") != std::string::npos) {
        std::string prefix = "/api/rooms/";
        std::string suffix = "/messages";
        int start = prefix.length();
        int end = path.find(suffix);
        int room_id = std::stoi(path.substr(start, end - start));
        auto msgs = message_service.list_messages(room_id);
        nlohmann::json arr = nlohmann::json::array();
        for (auto& m : msgs) {
            arr.push_back({
                {"user_id", m.user_id},
                {"content", m.content},
                {"sent_at", m.sent_at}
            });
        }
        res["success"] = true;
        res["messages"] = arr;
        send_json(connfd, res);
        return;
    }

if (method == "POST" && path == "/api/auth/logout") {
    nlohmann::json j = nlohmann::json::parse(body);
    std::string username = j["username"];
    user_service.logout(username); // 置 is_online=0、清空token等
    res["success"] = true;
    send_json(connfd, res);
    return;
}

    // 其它未知接口
    res["success"] = false;
    res["msg"] = "未知接口";
    send_json(connfd, res);
}



void HttpServer::send_json(int fd,const nlohmann::json& j){
    std::string body = j.dump();
    std::ostringstream oss;
    oss << "HTTP/1.1 200 OK\r\n"
        << "Content-Type: application/json\r\n"
        << "Content-Length: " << body.size() << "\r\n"
        << "Connection: keep-alive\r\n"
        << "Access-Control-Allow-Origin: *\r\n"
        << "Access-Control-Allow-Headers: Content-Type\r\n"
        << "Access-Control-Allow-Methods: *\r\n\r\n"
        << body;
    write(fd, oss.str().c_str(), oss.str().size());
}

