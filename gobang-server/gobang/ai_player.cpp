#include "ai_player.h"
#include <climits>
#include <vector>
#include <algorithm>
#include <random>

// 棋型枚举
enum PatternType {
    FIVE,      // 五连
    LIVE_FOUR, // 活四
    CHONG_FOUR,// 冲四
    LIVE_THREE,// 活三
    SLEEP_THREE,//眠三
    JUMP_LIVE_THREE,// 跳活三
    LIVE_TWO,  // 活二
    SLEEP_TWO  // 眠二
};

struct PatternStat {
    int five = 0, live_four = 0, 
    chong_four = 0, live_three = 0, 
    sleep_three = 0, jump_live_three = 0, 
    live_two = 0, sleep_two = 0;
    void reset() { five=live_four=chong_four=live_three=
        sleep_three=jump_live_three=live_two=sleep_two=0; }
};

// 各类棋型的权重
const int pattern_score[] = {
    1000000, // FIVE
    100000,  // LIVE_FOUR
    30000,   // CHONG_FOUR
    10000,   // LIVE_THREE
    3500,    // SLEEP_THREE
    8000,    // JUMP_LIVE_THREE
    500,     // LIVE_TWO
    120      // SLEEP_TWO
};

int search_depth(const std::string& level) {
    if (level == "easy") return 1;
    if (level == "normal") return 2;
    return 3; // hard
}

// 检查是否有n连
bool check_n(const std::array<std::array<int, 15>, 15>& board, int x, int y, int color, int n) {
    const int dirs[4][2] = {{1,0},{0,1},{1,1},{1,-1}};
    for (auto& d : dirs) {
        int cnt = 1;
        int nx = x + d[0], ny = y + d[1];
        while (nx>=0 && nx<15 && ny>=0 && ny<15 && board[ny][nx]==color) { ++cnt; nx += d[0]; ny += d[1]; }
        nx = x - d[0], ny = y - d[1];
        while (nx>=0 && nx<15 && ny>=0 && ny<15 && board[ny][nx]==color) { ++cnt; nx -= d[0]; ny -= d[1]; }
        if (cnt >= n) return true;
    }
    return false;
}



void analyze_point(const std::array<std::array<int,15>,15>& board, int x, int y, int color, PatternStat& stat) {
    const int dirs[4][2] = {{1,0},{0,1},{1,1},{1,-1}};
    for (auto& d : dirs) {
        std::vector<int> line;
        for (int i = -5; i <= 5; ++i) {
            int nx = x + i * d[0], ny = y + i * d[1];
            if (nx>=0 && nx<15 && ny>=0 && ny<15)
                line.push_back(board[ny][nx]);
            else
                line.push_back(-2); // 边界外
        }
        // 检查各种棋型
        for (int i = 1; i <= 5; ++i) {
            // 五连
            if (std::count(line.begin()+i, line.begin()+i+5, color)==5) { stat.five++; }
            // 活四：0 1 1 1 1 0
            if (line[i-1]==0 && std::count(line.begin()+i, line.begin()+i+4, color)==4 && line[i+4]==0) stat.live_four++;
            // 冲四
            if (((line[i-1]==-2 || line[i-1]==3-color) && std::count(line.begin()+i, line.begin()+i+4, color)==4 && line[i+4]==0)
             || (line[i-1]==0 && std::count(line.begin()+i, line.begin()+i+4, color)==4 && (line[i+4]==-2 || line[i+4]==3-color))) stat.chong_four++;
            // 活三
            if (line[i-1]==0 && std::count(line.begin()+i, line.begin()+i+3, color)==3 && line[i+3]==0
                && (line[i-2]==0 || line[i+4]==0)) stat.live_three++;
            // 跳活三
            if (line[i-1]==0 && line[i]==color && line[i+1]==color && line[i+2]==0 && line[i+3]==color && line[i+4]==0) stat.jump_live_three++;
            // 眠三
            if (((line[i-1]==-2 || line[i-1]==3-color) && std::count(line.begin()+i, line.begin()+i+3, color)==3 && line[i+3]==0)
             || (line[i-1]==0 && std::count(line.begin()+i, line.begin()+i+3, color)==3 && (line[i+3]==-2 || line[i+3]==3-color))) stat.sleep_three++;
            // 活二
            if (line[i-1]==0 && std::count(line.begin()+i, line.begin()+i+2, color)==2 && line[i+2]==0) stat.live_two++;
            // 眠二
            if (((line[i-1]==-2 || line[i-1]==3-color) && std::count(line.begin()+i, line.begin()+i+2, color)==2 && line[i+2]==0)
             || (line[i-1]==0 && std::count(line.begin()+i, line.begin()+i+2, color)==2 && (line[i+2]==-2 || line[i+2]==3-color))) stat.sleep_two++;
        }
    }
}
// 棋型扫描（只针对最后一个落子点）
// - 只对真实棋盘非空点做分析，统计当前局面
void scan_board(const std::array<std::array<int, 15>, 15>& board, int color, PatternStat& stat) {
    stat.reset();
    for (int y = 0; y < 15; ++y)
    for (int x = 0; x < 15; ++x)
        if (board[y][x] == color)
            analyze_point(board, x, y, color, stat);
}
// 更合理的局面评估（对双方实子全局棋型统计）
int evaluate(const std::array<std::array<int, 15>, 15>& board, int ai) {
    PatternStat ai_stat, opp_stat;
    scan_board(board, ai, ai_stat);
    scan_board(board, 3 - ai, opp_stat);
    int ai_score = 
        ai_stat.five     * pattern_score[FIVE] +
        ai_stat.live_four* pattern_score[LIVE_FOUR] +
        ai_stat.chong_four*pattern_score[CHONG_FOUR] +
        ai_stat.live_three*pattern_score[LIVE_THREE] +
        ai_stat.jump_live_three*pattern_score[JUMP_LIVE_THREE] +
        ai_stat.sleep_three*pattern_score[SLEEP_THREE] +
        ai_stat.live_two*pattern_score[LIVE_TWO] +
        ai_stat.sleep_two*pattern_score[SLEEP_TWO];
    int opp_score =
        opp_stat.five     * pattern_score[FIVE] +
        opp_stat.live_four* pattern_score[LIVE_FOUR] +
        opp_stat.chong_four*pattern_score[CHONG_FOUR] +
        opp_stat.live_three*pattern_score[LIVE_THREE] +
        opp_stat.jump_live_three*pattern_score[JUMP_LIVE_THREE] +
        opp_stat.sleep_three*pattern_score[SLEEP_THREE] +
        opp_stat.live_two*pattern_score[LIVE_TWO] +
        opp_stat.sleep_two*pattern_score[SLEEP_TWO];
    return ai_score*2 - opp_score*3;
}

// 候选点优先：周围两格有棋子
std::vector<std::pair<int,int>> candidate_moves(const std::array<std::array<int, 15>, 15>& board) {
    std::vector<std::pair<int,int>> moves;
    for (int y=0;y<15;++y) for (int x=0;x<15;++x) if (board[y][x]==0) {
        bool near = false;
        for (int dy=-2;dy<=2;++dy) for (int dx=-2;dx<=2;++dx) {
            int ny=y+dy, nx=x+dx;
            if (nx>=0&&nx<15&&ny>=0&&ny<15&&!(dx==0&&dy==0)&&board[ny][nx]!=0) near=true;
        }
        if (near) moves.emplace_back(x,y);
    }
    if (moves.empty()) moves.push_back({7,7}); // 空盘第一步
    std::shuffle(moves.begin(), moves.end(), std::mt19937{std::random_device{}()});
    return moves;
}

// 极大极小 + alpha-beta 剪枝
int minimax(std::array<std::array<int, 15>, 15>& board, int depth, bool is_ai, int ai, int& best_x, int& best_y, int alpha, int beta, const std::string& level) {
    int score = evaluate(board, ai);
    if (score >= 900000 || score <= -900000 || depth==0) return score;

    int best = is_ai ? INT_MIN : INT_MAX;
    auto moves = candidate_moves(board);
    std::vector<std::tuple<int,int,int>> scored;
    for (auto [x,y]: moves) {
        board[y][x] = is_ai ? ai : (3-ai);
        int sc = evaluate(board, ai);
        scored.emplace_back(sc, x, y);
        board[y][x] = 0;
    }
if (is_ai)
    std::sort(scored.begin(), scored.end(),
              [](auto &a, auto &b) { return std::get<0>(a) > std::get<0>(b); });
else
    std::sort(scored.begin(), scored.end(),
              [](auto &a, auto &b) { return std::get<0>(a) < std::get<0>(b); });

    int limit = (level=="easy" ? 4 : level=="normal" ? 8 : 14); // 扩展点数量限制
    for (size_t i=0;i<scored.size() && i<limit;++i) {
        int x=std::get<1>(scored[i]), y=std::get<2>(scored[i]);
        board[y][x] = is_ai ? ai : (3-ai);
        int dummy_x=-1,dummy_y=-1;
        int val = minimax(board, depth-1, !is_ai, ai, dummy_x, dummy_y, alpha, beta, level);
        board[y][x] = 0;
        if (is_ai) {
            if (val > best) { best = val; best_x = x; best_y = y; }
            alpha = std::max(alpha, val);
        } else {
            if (val < best) { best = val; }
            beta = std::min(beta, val);
        }
        if (beta <= alpha) break;
    }
    return best;
}

void get_ai_move(const std::array<std::array<int, 15>, 15>& board, const std::string& level, int& x, int& y) {
    int ai = 2; // 白
    // 1. 直接抓胜
    for (auto [xx, yy]: candidate_moves(board)) {
        auto b = board; b[yy][xx]=ai;
        if (check_n(b, xx, yy, ai, 5)) { x=xx; y=yy; return; }
    }
    // 2. 必须防守
    int opp = 1;
    for (auto [xx, yy]: candidate_moves(board)) {
        auto b = board; b[yy][xx]=opp;
        if (check_n(b, xx, yy, opp, 5)) { x=xx; y=yy; return; }
    }
    // 3. 极大极小 + alpha-beta
    int depth = search_depth(level);
    int best_x = -1, best_y = -1;
    std::array<std::array<int, 15>, 15> board_copy = board;
    minimax(board_copy, depth, true, ai, best_x, best_y, INT_MIN, INT_MAX, level);
    if (best_x < 0 || best_y < 0) {
        auto cands = candidate_moves(board);
        x = cands[0].first; y = cands[0].second;
    } else {
        x = best_x; y = best_y;
    }
}
