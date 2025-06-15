#pragma once
#include <string>
#include <charconv>
#include <nlohmann/json.hpp>
#include <sstream>
#include <sys/socket.h>

// ---------- safe string -> int ----------
inline int safe_to_int(std::string_view sv, int def = -1) {
    int v = def;
    auto [p, ec] = std::from_chars(sv.data(), sv.data() + sv.size(), v);
    return (ec == std::errc()) ? v : def;
}

// ---------- send 400 ----------
inline void send_400(int fd, const std::string& msg) {
    std::ostringstream oss;
    oss << "HTTP/1.1 400 Bad Request\r\n"
        << "Content-Type: text/plain\r\n"
        << "Content-Length: " << msg.size() << "\r\n\r\n"
        << "Connection: close\r\n\r\n"
        << msg;
    ::send(fd, oss.str().c_str(), oss.str().size(), 0);
}

// ---------- parse JSON, allow_exceptions = false ----------
inline bool safe_json_parse(const std::string& body,
                            nlohmann::json& j_out) {
    j_out = nlohmann::json::parse(body, /*cb=*/nullptr,
                                  /*allow_exceptions=*/false);
    return !j_out.is_discarded();
}
