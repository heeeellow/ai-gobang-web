#pragma once
#include <string>
#include <random>
#include <chrono>

inline std::string gen_token() {
    auto now = std::chrono::system_clock::now().time_since_epoch().count();
    static std::mt19937_64 rng(now);
    std::string charset = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    std::string token;
    for (int i = 0; i < 32; ++i)
        token += charset[rng() % charset.size()];
    return token;
}
