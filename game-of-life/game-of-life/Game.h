#ifndef GAME_H
#define GAME_H

#include <windows.h>

// 常量定义
const int GW = 64;      // 网格宽度
const int GH = 64;      // 网格高度
const int CS = 10;      // 细胞大小
const int GM = 5;       // 网格边距
const int UIH = 45;     // UI区域高度

// 窗口尺寸
const int WW = GW * CS + GM * 2;
const int WH = GH * CS + GM * 2 + UIH + 20;

const int TID = 1;      // 定时器ID
const int TINT = 65;    // 每65ms更新一代

// 游戏类 - 管理游戏状态和逻辑
class Game {
private:
    bool grid[GH][GW];   // 当前世代细胞状态
    bool next[GH][GW];   // 下一世代细胞状态
    bool running;        // 游戏运行状态

public:
    Game();
    void init();                    // 初始化所有细胞为死亡状态
    void update();                  // 更新到下一代
    int getNeighbors(int r, int c) const;  // 计算存活邻居数
    void randomize();               // 随机初始化（30%存活率）
    void clear();                   // 清空所有细胞
    void toggleCell(int r, int c);  // 切换细胞状态
    bool isRunning() const { return running; }
    void setRunning(bool run) { running = run; }
    bool getCell(int r, int c) const { return grid[r][c]; }
    int getAliveCount() const;      // 获取存活细胞数量
};

#endif // GAME_H