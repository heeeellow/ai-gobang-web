#pragma once
#include <string>
#include <nlohmann/json.hpp>

bool websocket_handshake(int fd);
bool ws_recv_text(int fd, std::string& out);
bool ws_send_text(int fd, const std::string& text);

