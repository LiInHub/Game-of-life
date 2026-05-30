#ifndef RENDERER_H
#define RENDERER_H

#include <windows.h>
#include "Game.h"

// 颜色常量
extern const COLORREF CLIVE;   // 存活细胞颜色
extern const COLORREF CDEAD;   // 死亡细胞颜色
extern const COLORREF CGRID;   // 网格线颜色
extern const COLORREF CBG;     // 背景色
extern const COLORREF CTEXT;   // 文字颜色

// 渲染器类 - 负责所有绘制操作
class Renderer {
private:
    bool showGridLine;  // 是否显示网格线

public:
    Renderer();
    void setShowGridLine(bool show) { showGridLine = show; }
    bool getShowGridLine() const { return showGridLine; }
    void toggleGridLine() { showGridLine = !showGridLine; }

    void drawGridAndCells(HDC hdc, const Game& game);
    void drawUI(HWND hwnd, HDC hdc, const Game& game);

    // 将屏幕坐标转换为网格坐标
    bool screenToGrid(int x, int y, int& row, int& col) const;
};

#endif // RENDERER_H