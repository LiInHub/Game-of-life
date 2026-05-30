#include "Renderer.h"

// 颜色定义
const COLORREF CLIVE = RGB(0, 250, 100);   // 存活细胞颜色（绿色）
const COLORREF CDEAD = RGB(0, 0, 0);       // 死亡细胞颜色（黑色）
const COLORREF CGRID = RGB(50, 50, 60);    // 网格线颜色
const COLORREF CBG = RGB(0, 0, 0);         // 背景色
const COLORREF CTEXT = RGB(250, 250, 250); // 文字颜色（白）

Renderer::Renderer() : showGridLine(true) {}

void Renderer::drawGridAndCells(HDC hdc, const Game& game) {
    HBRUSH bl = CreateSolidBrush(CLIVE);
    HBRUSH bd = CreateSolidBrush(CDEAD);
    HPEN p = CreatePen(PS_SOLID, 1, CGRID);

    int xs = GM, ys = GM;

    for (int y = 0; y < GH; y++) {
        for (int x = 0; x < GW; x++) {
            RECT rect{
                xs + x * CS,
                ys + y * CS,
                xs + (x + 1) * CS,
                ys + (y + 1) * CS
            };

            FillRect(hdc, &rect, game.getCell(y, x) ? bl : bd);

            if (showGridLine) {
                SelectObject(hdc, p);
                MoveToEx(hdc, rect.left, rect.top, 0);
                LineTo(hdc, rect.right, rect.top);
                LineTo(hdc, rect.right, rect.bottom);
                LineTo(hdc, rect.left, rect.bottom);
                LineTo(hdc, rect.left, rect.top);
            }
        }
    }

    DeleteObject(bl);
    DeleteObject(bd);
    DeleteObject(p);
}

void Renderer::drawUI(HWND hwnd, HDC hdc, const Game& game) {
    SetBkMode(hdc, TRANSPARENT);
    SetTextColor(hdc, CTEXT);

    HFONT font = CreateFont(16, 0, 0, 0, 400, 0, 0, 0,
        1, 0, 0, 0, 0, L"Consolas");
    HFONT oldFont = (HFONT)SelectObject(hdc, font);

    RECT rc;
    GetClientRect(hwnd, &rc);
    int y = GM + GH * CS + 8;
    int rightX = max(rc.right - 100, 150);

    wchar_t buf[256];
    wsprintfW(buf, game.isRunning() ?
        L"运行中 | 空格：暂停 | R：随机 | C：清空 | G：网格" :
        L"已暂停 | 空格：开始 | R：随机 | C：清空 | G：网格");
    TextOutW(hdc, 10, y, buf, lstrlenW(buf));

    wsprintfW(buf, L"存活: %d", game.getAliveCount());
    TextOutW(hdc, rightX, y, buf, lstrlenW(buf));

    SelectObject(hdc, oldFont);
    DeleteObject(font);
}

bool Renderer::screenToGrid(int x, int y, int& row, int& col) const {
    int gx = GM, gy = GM;
    int gw = GW * CS, gh = GH * CS;

    if (x >= gx && x < gx + gw && y >= gy && y < gy + gh) {
        col = (x - gx) / CS;
        row = (y - gy) / CS;
        return (row >= 0 && row < GH && col >= 0 && col < GW);
    }
    return false;
}