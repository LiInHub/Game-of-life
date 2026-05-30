#include "Game.h"
#include <cstdlib>

Game::Game() : running(false) {
    init();
}

void Game::init() {
    for (int y = 0; y < GH; y++)
        for (int x = 0; x < GW; x++)
            grid[y][x] = false;
}

void Game::update() {
    // 计算下一代
    for (int y = 0; y < GH; y++)
        for (int x = 0; x < GW; x++) {
            int n = getNeighbors(y, x);
            next[y][x] = grid[y][x] ? (n == 2 || n == 3) : (n == 3);
        }

    // 复制到当前代
    for (int y = 0; y < GH; y++)
        for (int x = 0; x < GW; x++)
            grid[y][x] = next[y][x];
}

int Game::getNeighbors(int r, int c) const {
    int cnt = 0;
    for (int dr = -1; dr <= 1; dr++)
        for (int dc = -1; dc <= 1; dc++) {
            if (dr == 0 && dc == 0) continue;
            int nr = r + dr, nc = c + dc;
            if (nr >= 0 && nr < GH && nc >= 0 && nc < GW && grid[nr][nc])
                cnt++;
        }
    return cnt;
}

void Game::randomize() {
    for (int y = 0; y < GH; y++)
        for (int x = 0; x < GW; x++)
            grid[y][x] = (rand() % 100) < 30;
}

void Game::clear() {
    for (int y = 0; y < GH; y++)
        for (int x = 0; x < GW; x++)
            grid[y][x] = false;
}

void Game::toggleCell(int r, int c) {
    if (r >= 0 && r < GH && c >= 0 && c < GW)
        grid[r][c] = !grid[r][c];
}

int Game::getAliveCount() const {
    int cnt = 0;
    for (int i = 0; i < GH; i++)
        for (int j = 0; j < GW; j++)
            if (grid[i][j]) cnt++;
    return cnt;
}