#include <windows.h>
#include <cstdlib>
#include <ctime>
#include <cstdio>

//常量
const int GW = 64, GH = 64;    // 网格宽度和高度
const int CS = 10;              // 细胞大小
const int GM = 5;               // 网格边距
const int UIH = 45;             // UI区域高度

// 窗口尺寸
const int WW = GW * CS + GM * 2;                 // 窗口宽度 = 网格宽度 + 左右边距
const int WH = GH * CS + GM * 2 + UIH + 20;      // 窗口高度 = 网格高度 + 上下边距 + UI区域

const int TID = 1;      // 定时器ID
const int TINT = 100;    // 定时器间隔(毫秒)，每100ms更新一代

bool grid[GH][GW];      // 当前世代细胞状态
bool next[GH][GW];      // 下一世代细胞状态（双缓冲）
bool run = false;       // 游戏运行状态，true=自动演化
bool gridLine = true;   // 是否显示网格线

// 颜色
const COLORREF
CLIVE = RGB(0, 250, 100),   // 存活细胞颜色（绿色）
CDEAD = RGB(0, 0, 0),    // 死亡细胞颜色（黑色）
CGRID = RGB(50, 50, 60),    // 网格线颜色
CBG = RGB(0, 0, 0),      // 背景色
CTEXT = RGB(250, 250, 250); // 文字颜色（白）

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
void Init();                    // 初始化所有细胞为死亡状态
void Update();                  // 更新到下一代（应用康威规则）
int Neighbors(int, int);        // 计算指定细胞的存活邻居数
void Draw(HDC);                 // 绘制网格和细胞
void DrawUI(HWND, HDC);         // 绘制UI文字
void Rand();                    // 随机生成初始状态（30%存活率）
void Clear();                   // 清空所有细胞

//主函数
int main() {
    srand(time(0));  //随机数种子

    HINSTANCE hIns = GetModuleHandle(0);
    const wchar_t cls[] = L"GameOfLife";

    // 注册窗口类
    WNDCLASS wc{};
    wc.lpfnWndProc = WndProc;      // 窗口过程函数
    wc.hInstance = hIns;
    wc.lpszClassName = cls;
    wc.hCursor = LoadCursor(0, IDC_ARROW);  // 默认箭头光标
    wc.hbrBackground = CreateSolidBrush(CBG); // 背景画刷
    RegisterClass(&wc);

    // 创建窗口
    HWND hwnd = CreateWindowEx(0, cls, L"康威生命游戏 64x64",
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
        CW_USEDEFAULT, CW_USEDEFAULT, WW, WH, 0, 0, hIns, 0);

    if (!hwnd) return 0;
    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    // 消息循环
    MSG msg{};
    while (GetMessage(&msg, 0, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return 0;
}

//窗口过程
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    switch (msg) {
    case WM_CREATE:
        Init();                     // 初始化游戏网格
        SetTimer(hwnd, TID, TINT, 0);  // 启动定时器
        break;

    case WM_TIMER:
        if (run) {                  // 仅在运行状态下更新
            Update();
            InvalidateRect(hwnd, 0, 0);  // 触发重绘
        }
        break;

    case WM_PAINT: {
        // 双缓冲绘制，避免闪烁
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        HDC mdc = CreateCompatibleDC(hdc);  // 内存DC
        RECT rc;
        GetClientRect(hwnd, &rc);
        int w = rc.right, h = rc.bottom;

        // 创建兼容位图
        HBITMAP bmp = CreateCompatibleBitmap(hdc, w, h);
        HBITMAP old = (HBITMAP)SelectObject(mdc, bmp);

        // 绘制背景、网格和UI
        FillRect(mdc, &rc, CreateSolidBrush(CBG));
        Draw(mdc);
        DrawUI(hwnd, mdc);

        // 一次性复制到屏幕
        BitBlt(hdc, 0, 0, w, h, mdc, 0, 0, SRCCOPY);

        // 清理资源
        SelectObject(mdc, old);
        DeleteObject(bmp);
        DeleteDC(mdc);
        EndPaint(hwnd, &ps);
        break;
    }

    case WM_LBUTTONDOWN: {
        // 鼠标点击切换细胞状态
        int x = LOWORD(lp), y = HIWORD(lp);  // 获取点击坐标
        int gx = GM, gy = GM;
        int gw = GW * CS, gh = GH * CS;

        // 判断点击是否在网格区域内
        if (x >= gx && x < gx + gw && y >= gy && y < gy + gh) {
            int c = (x - gx) / CS;  // 列索引
            int r = (y - gy) / CS;  // 行索引
            if (r >= 0 && r < GH && c >= 0 && c < GW) {
                grid[r][c] = !grid[r][c];  // 翻转状态
                InvalidateRect(hwnd, 0, 0); // 重绘
            }
        }
        break;
    }

    case WM_KEYDOWN:
        // 控制器
        switch (wp) {
        case VK_SPACE: run = !run; break;      // 空格：运行/暂停
        case 'R': Rand(); break;                // R：随机初始化
        case 'C': Clear(); break;               // C：清空网格
        case 'G': gridLine = !gridLine; break;  // G：切换网格线
        }
        InvalidateRect(hwnd, 0, 0);
        break;

    case WM_DESTROY:
        KillTimer(hwnd, TID);   // 停止定时器
        PostQuitMessage(0);     // 退出消息循环
        break;

    default:
        return DefWindowProc(hwnd, msg, wp, lp);
    }
    return 0;
}
//游戏逻辑

// 初始化：所有细胞死亡
void Init() {
    for (int y = 0; y < GH; y++)
        for (int x = 0; x < GW; x++)
            grid[y][x] = 0;
}

void Update() {
    // 计算下一代状态
    for (int y = 0; y < GH; y++)
        for (int x = 0; x < GW; x++) {
            int n = Neighbors(y, x);
            next[y][x] = grid[y][x] ? (n == 2 || n == 3) : (n == 3);
        }

    // 复制到当前代
    for (int y = 0; y < GH; y++)
        for (int x = 0; x < GW; x++)
            grid[y][x] = next[y][x];
}

// 计算细胞(r,c)的存活邻居数
int Neighbors(int r, int c) {
    int cnt = 0;
    for (int dr = -1; dr <= 1; dr++)
        for (int dc = -1; dc <= 1; dc++) {
            if (!dr && !dc) continue;  // 跳过自身
            int nr = r + dr, nc = c + dc;
            // 边界检查，只统计网格内的细胞
            if (nr >= 0 && nr < GH && nc >= 0 && nc < GW && grid[nr][nc])
                cnt++;
        }
    return cnt;
}

// 绘制网格和细胞
void Draw(HDC hdc) {
    HBRUSH bl = CreateSolidBrush(CLIVE);  // 存活细胞画刷
    HBRUSH bd = CreateSolidBrush(CDEAD);  // 死亡细胞画刷
    HPEN p = CreatePen(PS_SOLID, 1, CGRID); // 网格线画笔
    int xs = GM, ys = GM;  // 网格起始坐标

    for (int y = 0; y < GH; y++)
        for (int x = 0; x < GW; x++) {
            // 计算细胞矩形区域
            RECT r{
                xs + x * CS,
                ys + y * CS,
                xs + (x + 1) * CS,
                ys + (y + 1) * CS
            };
            // 根据细胞状态填充颜色
            FillRect(hdc, &r, grid[y][x] ? bl : bd);

            // 绘制网格线（可选）
            if (gridLine) {
                SelectObject(hdc, p);
                MoveToEx(hdc, r.left, r.top, 0);
                LineTo(hdc, r.right, r.top);
                LineTo(hdc, r.right, r.bottom);
                LineTo(hdc, r.left, r.bottom);
                LineTo(hdc, r.left, r.top);
            }
        }

    // 清理GDI资源
    DeleteObject(bl);
    DeleteObject(bd);
    DeleteObject(p);
}

// 游戏状态提示、存活细胞计数
void DrawUI(HWND hwnd, HDC hdc) {
    SetBkMode(hdc, TRANSPARENT);   // 文字背景透明
    SetTextColor(hdc, CTEXT);

    // 创建字体
    HFONT f = CreateFont(16, 0, 0, 0, 400, 0, 0, 0, 1, 0, 0, 0, 0, L"Consolas");
    HFONT old = (HFONT)SelectObject(hdc, f);

    RECT rc;
    GetClientRect(hwnd, &rc);
    int y = GM + GH * CS + 8;  // UI文字Y坐标（网格下方）
    int rx = max(rc.right - 100, 150);  // 右侧文字X坐标

    wchar_t buf[256];
    wsprintfW(buf, run ?
        L"运行中 | 空格：暂停 | R：随机 | C：清空 | G：网格" :
        L"已暂停 | 空格：开始 | R：随机 | C：清空 | G：网格");
    TextOutW(hdc, 10, y, buf, lstrlenW(buf));

    // 统计并显示存活细胞数量
    int cnt = 0;
    for (int i = 0; i < GH; i++)
        for (int j = 0; j < GW; j++)
            if (grid[i][j]) cnt++;
    wsprintfW(buf, L"存活: %d", cnt);
    TextOutW(hdc, rx, y, buf, lstrlenW(buf));

    // 恢复旧字体并删除创建的字体
    SelectObject(hdc, old);
    DeleteObject(f);
}

// 随机初始化：细胞有30%概率存活
void Rand() {
    for (int y = 0; y < GH; y++)
        for (int x = 0; x < GW; x++)
            grid[y][x] = rand() % 100 < 30;
}

// 清空所有细胞
void Clear() {
    for (int y = 0; y < GH; y++)
        for (int x = 0; x < GW; x++)
            grid[y][x] = 0;
}
