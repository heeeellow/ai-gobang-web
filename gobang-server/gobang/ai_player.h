// ai_player.h
#pragma once
#include <array>
#include <string>
void get_ai_move(const std::array<std::array<int, 15>, 15>& board, const std::string& level, int& x, int& y);
