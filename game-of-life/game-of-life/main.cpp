#include <windows.h>
#include <cstdlib>
#include <ctime>
#include "Game.h"
#include "Renderer.h"

// 全局对象
Game g_game;
Renderer g_renderer;

// 窗口过程声明
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

// 主函数
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
    LPSTR lpCmdLine, int nCmdShow) {
    srand(static_cast<unsigned>(time(nullptr)));

    const wchar_t cls[] = L"GameOfLife";

    WNDCLASS wc{};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = cls;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = CreateSolidBrush(CBG);
    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(0, cls, L"康威生命游戏 64x64",
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
        CW_USEDEFAULT, CW_USEDEFAULT, WW, WH,
        nullptr, nullptr, hInstance, nullptr);

    if (!hwnd) return 0;
    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    MSG msg{};
    while (GetMessage(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return 0;
}

// 窗口过程
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    switch (msg) {
    case WM_CREATE:
        g_game.init();
        SetTimer(hwnd, TID, TINT, nullptr);
        break;

    case WM_TIMER:
        if (g_game.isRunning()) {
            g_game.update();
            InvalidateRect(hwnd, nullptr, false);
        }
        break;

    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);

        // 双缓冲
        HDC memDC = CreateCompatibleDC(hdc);
        RECT rc;
        GetClientRect(hwnd, &rc);
        int w = rc.right, h = rc.bottom;

        HBITMAP bitmap = CreateCompatibleBitmap(hdc, w, h);
        HBITMAP oldBitmap = (HBITMAP)SelectObject(memDC, bitmap);

        FillRect(memDC, &rc, CreateSolidBrush(CBG));
        g_renderer.drawGridAndCells(memDC, g_game);
        g_renderer.drawUI(hwnd, memDC, g_game);

        BitBlt(hdc, 0, 0, w, h, memDC, 0, 0, SRCCOPY);

        SelectObject(memDC, oldBitmap);
        DeleteObject(bitmap);
        DeleteDC(memDC);
        EndPaint(hwnd, &ps);
        break;
    }

    case WM_LBUTTONDOWN: {
        int x = LOWORD(lp), y = HIWORD(lp);
        int row, col;
        if (g_renderer.screenToGrid(x, y, row, col)) {
            g_game.toggleCell(row, col);
            InvalidateRect(hwnd, nullptr, false);
        }
        break;
    }

    case WM_KEYDOWN:
        switch (wp) {
        case VK_SPACE:
            g_game.setRunning(!g_game.isRunning());
            break;
        case 'R':
            g_game.randomize();
            break;
        case 'C':
            g_game.clear();
            break;
        case 'G':
            g_renderer.toggleGridLine();
            break;
        }
        InvalidateRect(hwnd, nullptr, false);
        break;

    case WM_DESTROY:
        KillTimer(hwnd, TID);
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProc(hwnd, msg, wp, lp);
    }
    return 0;
}