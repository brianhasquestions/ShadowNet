#include "melt_window.h"
#include "../core/game.h"
#include <strsafe.h>
#include <cstdlib>
#include <ctime>

// ============================================================================
// Screen Melt State
// ============================================================================

HWND g_MeltWnd = NULL;
static HDC g_MeltDC = NULL;
static HBITMAP g_MeltBitmap = NULL;
static int g_MeltWidth = 0;
static int g_MeltHeight = 0;
static int g_MeltTickCount = 0;
static int g_MeltPhase = 0;
static HFONT g_MeltFont = NULL;
static wchar_t g_MeltAttackerName[32] = {};

static MatrixColumn g_MatrixCols[MELT_MAX_MATRIX_COLS];
static int g_NumMatrixCols = 0;

// ============================================================================
// Phase Transition Tick Counts
// ============================================================================

constexpr int kGlitchEndTick = 120;       // ~2s glitch
constexpr int kMeltEndTick = 1200;        // ~18s melt
constexpr int kMatrixEndTick = 3000;      // ~30s matrix rain
constexpr int kGlitchTearCount = 8;
constexpr int kGlitchRGBInterval = 8;
constexpr int kGlitchTextStartTick = 60;
constexpr int kGlitchTextToggleInterval = 10;
constexpr int kMeltStripCount = 15;
constexpr int kMeltMessageInterval = 60;
constexpr int kMeltAttackerNameInterval = 180;
constexpr int kMatrixDarkenInterval = 3;
constexpr int kMatrixHackedTextInterval = 90;
constexpr int kFadeBlocksPerProgress = 2;
constexpr int kFadeMaxBlocks = 200;
constexpr int kMeltMessageCount = 12;
constexpr int kMatrixMinSpeed = 8;
constexpr int kMatrixMaxSpeed = 16;
constexpr int kCharMutateChance = 20;
constexpr int kCharTypeCount = 4;

// ============================================================================
// Matrix Rain Initialization
// ============================================================================

static void InitMatrixRain(int screenW, int screenH)
{
    g_NumMatrixCols = screenW / MELT_COL_WIDTH_PX;
    if (g_NumMatrixCols > MELT_MAX_MATRIX_COLS)
    {
        g_NumMatrixCols = MELT_MAX_MATRIX_COLS;
    }

    constexpr int kColXJitter = 6;
    constexpr int kMinLength = 10;
    constexpr int kLengthRange = 25;

    for (int i = 0; i < g_NumMatrixCols; i++)
    {
        g_MatrixCols[i].x = i * MELT_COL_WIDTH_PX + (rand() % kColXJitter);
        g_MatrixCols[i].y = -(rand() % screenH);
        g_MatrixCols[i].speed = kMatrixMinSpeed + (rand() % kMatrixMaxSpeed);
        g_MatrixCols[i].length = kMinLength + (rand() % kLengthRange);
        for (int j = 0; j < MELT_MATRIX_CHAR_LEN; j++)
        {
            int r = rand() % kCharTypeCount;
            if (0 == r)
            {
                g_MatrixCols[i].chars[j] = L'0' + (rand() % 10);
            }
            else if (1 == r)
            {
                g_MatrixCols[i].chars[j] = L'A' + (rand() % 26);
            }
            else if (2 == r)
            {
                g_MatrixCols[i].chars[j] = static_cast<wchar_t>(0x30A0 + (rand() % 96));
            }
            else
            {
                g_MatrixCols[i].chars[j] = L"@#$%&*!?<>{}[]|/\\"[rand() % 18];
            }
        }
    }
}

// ============================================================================
// Screen Melt Window Procedure
// ============================================================================

LRESULT CALLBACK MeltWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
        case WM_CREATE:
        {
            HDC hdcScreen = GetDC(NULL);
            g_MeltWidth = GetSystemMetrics(SM_CXSCREEN);
            g_MeltHeight = GetSystemMetrics(SM_CYSCREEN);

            g_MeltDC = CreateCompatibleDC(hdcScreen);
            g_MeltBitmap = CreateCompatibleBitmap(hdcScreen, g_MeltWidth, g_MeltHeight);
            SelectObject(g_MeltDC, g_MeltBitmap);

            BitBlt(g_MeltDC, 0, 0, g_MeltWidth, g_MeltHeight, hdcScreen, 0, 0, SRCCOPY);
            ReleaseDC(NULL, hdcScreen);

            constexpr int kMeltFontSize = 14;
            g_MeltFont = CreateFontW(kMeltFontSize, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
                DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                CLEARTYPE_QUALITY, FIXED_PITCH | FF_MODERN, L"Consolas");

            g_MeltTickCount = 0;
            g_MeltPhase = 0;
            srand(static_cast<unsigned>(time(NULL)));
            InitMatrixRain(g_MeltWidth, g_MeltHeight);

            SetTimer(hWnd, TIMER_MELT, MELT_TICK_MS, NULL);
            return 0;
        }

        case WM_TIMER:
        {
            if (TIMER_MELT != wParam)
            {
                break;
            }

            g_MeltTickCount++;

            // Phase transitions
            if (g_MeltTickCount < kGlitchEndTick)
            {
                g_MeltPhase = 0;
            }
            else if (g_MeltTickCount < kMeltEndTick)
            {
                g_MeltPhase = 1;
            }
            else if (g_MeltTickCount < kMatrixEndTick)
            {
                g_MeltPhase = 2;
            }
            else
            {
                g_MeltPhase = 3;
            }

            if (g_MeltTickCount >= MELT_DURATION_TICKS)
            {
                KillTimer(hWnd, TIMER_MELT);
                DestroyWindow(hWnd);
                return 0;
            }

            // === PHASE 0: Glitch Effect ===
            if (0 == g_MeltPhase)
            {
                for (int i = 0; i < kGlitchTearCount; i++)
                {
                    int x = rand() % g_MeltWidth;
                    int y = rand() % g_MeltHeight;
                    int w = 20 + rand() % 300;
                    int h = 1 + rand() % 15;
                    int srcY = rand() % g_MeltHeight;

                    BitBlt(g_MeltDC, x, y, w, h, g_MeltDC, x + (rand() % 40 - 20), srcY, SRCCOPY);
                }

                if (0 == g_MeltTickCount % kGlitchRGBInterval)
                {
                    int shiftX = rand() % g_MeltWidth;
                    int shiftY = rand() % g_MeltHeight;
                    int shiftW = 50 + rand() % 200;
                    int shiftH = 10 + rand() % 50;
                    constexpr int kRGBShiftOffset = 3;
                    BitBlt(g_MeltDC, shiftX + kRGBShiftOffset, shiftY, shiftW, shiftH, g_MeltDC, shiftX, shiftY, SRCAND);
                }

                if (g_MeltTickCount > kGlitchTextStartTick && 0 == (g_MeltTickCount / kGlitchTextToggleInterval) % 2)
                {
                    constexpr int kIntrusionFontSize = 48;
                    constexpr int kIntrusionTextOffsetX = 280;
                    constexpr int kIntrusionTextOffsetY = 30;
                    HFONT bigFont = CreateFontW(kIntrusionFontSize, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
                        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                        CLEARTYPE_QUALITY, FIXED_PITCH | FF_MODERN, L"Consolas");
                    HFONT old = static_cast<HFONT>(SelectObject(g_MeltDC, bigFont));
                    SetBkMode(g_MeltDC, TRANSPARENT);
                    SetTextColor(g_MeltDC, RGB(255, 0, 0));
                    int tx = g_MeltWidth / 2 - kIntrusionTextOffsetX;
                    int ty = g_MeltHeight / 2 - kIntrusionTextOffsetY;
                    TextOutW(g_MeltDC, tx, ty, L"!! INTRUSION DETECTED !!", 24);
                    SelectObject(g_MeltDC, old);
                    DeleteObject(bigFont);
                }
            }

            // === PHASE 1: Melt Effect ===
            if (1 == g_MeltPhase)
            {
                for (int i = 0; i < kMeltStripCount; i++)
                {
                    int x = rand() % g_MeltWidth;
                    int w = 2 + rand() % 140;
                    int h = g_MeltHeight;
                    int drop = 1 + rand() % 6;

                    BitBlt(g_MeltDC, x, drop, w, h - drop, g_MeltDC, x, 0, SRCCOPY);
                }

                if (0 == g_MeltTickCount % kMeltMessageInterval)
                {
                    HFONT old = static_cast<HFONT>(SelectObject(g_MeltDC, g_MeltFont));
                    SetBkMode(g_MeltDC, TRANSPARENT);

                    const wchar_t* messages[] =
                    {
                        L"> ACCESSING YOUR SYSTEMS...",
                        L"> DOWNLOADING WALLET DATA...",
                        L"> EXFILTRATING CREDENTIALS...",
                        L"> PLANTING BACKDOOR...",
                        L"> WIPING AUDIT LOGS...",
                        L"> ENCRYPTING YOUR FILES...",
                        L"> STEALING CRYPTO KEYS...",
                        L"> HIJACKING C2 CHANNELS...",
                        L"> DUMPING MEMORY...",
                        L"> KEYLOGGER INSTALLED...",
                        L"> PIVOTING TO INTERNAL NETWORK...",
                        L"> LATERAL MOVEMENT IN PROGRESS...",
                    };
                    int msgIdx = (g_MeltTickCount / kMeltMessageInterval) % kMeltMessageCount;
                    constexpr int kMsgMargin = 20;
                    constexpr int kMsgAreaPadding = 400;
                    constexpr int kShadowOffset = 2;
                    int mx = kMsgMargin + rand() % (g_MeltWidth - kMsgAreaPadding);
                    int my = kMsgMargin + rand() % (g_MeltHeight - (kMsgMargin * 2));

                    SetTextColor(g_MeltDC, RGB(0, 0, 0));
                    TextOutW(g_MeltDC, mx + kShadowOffset, my + kShadowOffset, messages[msgIdx], lstrlenW(messages[msgIdx]));
                    SetTextColor(g_MeltDC, RGB(0, 255, 0));
                    TextOutW(g_MeltDC, mx, my, messages[msgIdx], lstrlenW(messages[msgIdx]));

                    SelectObject(g_MeltDC, old);
                }

                if (0 == g_MeltTickCount % kMeltAttackerNameInterval && L'\0' != g_MeltAttackerName[0])
                {
                    constexpr int kAttackerFontSize = 36;
                    constexpr int kAttackerTextOffsetX = 200;
                    constexpr int kAttackerTextOffsetY = 20;
                    constexpr int kAttackerJitter = 100;
                    constexpr int kAttackerShadowOffset = 2;
                    HFONT bigFont = CreateFontW(kAttackerFontSize, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
                        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                        CLEARTYPE_QUALITY, FIXED_PITCH | FF_MODERN, L"Consolas");
                    HFONT old = static_cast<HFONT>(SelectObject(g_MeltDC, bigFont));
                    SetBkMode(g_MeltDC, TRANSPARENT);

                    wchar_t buf[BUF_XLARGE] = {};
                    StringCchPrintfW(buf, BUF_XLARGE, L"[ %s PWNED YOU ]", g_MeltAttackerName);

                    int tx = g_MeltWidth / 2 - kAttackerTextOffsetX;
                    int ty = g_MeltHeight / 2 - kAttackerTextOffsetY + (rand() % kAttackerJitter - kAttackerJitter / 2);

                    SetTextColor(g_MeltDC, RGB(0, 0, 0));
                    TextOutW(g_MeltDC, tx + kAttackerShadowOffset, ty + kAttackerShadowOffset, buf, lstrlenW(buf));
                    SetTextColor(g_MeltDC, RGB(255, 50, 50));
                    TextOutW(g_MeltDC, tx, ty, buf, lstrlenW(buf));

                    SelectObject(g_MeltDC, old);
                    DeleteObject(bigFont);
                }
            }

            // === PHASE 2: Matrix Rain ===
            if (2 == g_MeltPhase)
            {
                if (0 == g_MeltTickCount % kMatrixDarkenInterval)
                {
                    for (int sy = 0; sy < g_MeltHeight; sy += 2)
                    {
                        for (int sx = 0; sx < g_MeltWidth; sx += 2)
                        {
                            SetPixel(g_MeltDC, sx, sy, RGB(0, 0, 0));
                        }
                    }
                }

                HFONT old = static_cast<HFONT>(SelectObject(g_MeltDC, g_MeltFont));
                SetBkMode(g_MeltDC, TRANSPARENT);

                constexpr int kCharHeight = 16;
                constexpr int kHeadGreen = 255;
                constexpr int kGreenDecayPerChar = 12;
                constexpr int kMinGreen = 30;
                constexpr int kResetYOffset = 300;

                for (int i = 0; i < g_NumMatrixCols; i++)
                {
                    MatrixColumn& col = g_MatrixCols[i];
                    col.y += col.speed;

                    for (int j = 0; j < col.length && j < MELT_MATRIX_CHAR_LEN; j++)
                    {
                        int drawY = col.y - j * kCharHeight;
                        if (drawY < 0 || drawY >= g_MeltHeight)
                        {
                            continue;
                        }

                        if (0 == j)
                        {
                            SetTextColor(g_MeltDC, RGB(200, kHeadGreen, 200));
                        }
                        else
                        {
                            int green = kHeadGreen - (j * kGreenDecayPerChar);
                            if (green < kMinGreen)
                            {
                                green = kMinGreen;
                            }
                            SetTextColor(g_MeltDC, RGB(0, green, 0));
                        }

                        wchar_t ch[2] = { col.chars[j], 0 };
                        TextOutW(g_MeltDC, col.x, drawY, ch, 1);

                        if (0 == rand() % kCharMutateChance)
                        {
                            int r = rand() % kCharTypeCount;
                            if (0 == r)
                            {
                                col.chars[j] = L'0' + (rand() % 10);
                            }
                            else if (1 == r)
                            {
                                col.chars[j] = L'A' + (rand() % 26);
                            }
                            else if (2 == r)
                            {
                                col.chars[j] = static_cast<wchar_t>(0x30A0 + (rand() % 96));
                            }
                            else
                            {
                                col.chars[j] = L"@#$%&*!?<>{}[]|/\\"[rand() % 18];
                            }
                        }
                    }

                    if (col.y - col.length * kCharHeight > g_MeltHeight)
                    {
                        col.y = -(rand() % kResetYOffset);
                        col.speed = kMatrixMinSpeed + (rand() % kMatrixMaxSpeed);
                    }
                }

                SelectObject(g_MeltDC, old);

                if (0 == (g_MeltTickCount / kMatrixHackedTextInterval) % 2)
                {
                    constexpr int kHackedFontSize = 60;
                    constexpr int kHackedTextOffsetX = 340;
                    constexpr int kHackedTextOffsetY = 35;
                    HFONT bigFont = CreateFontW(kHackedFontSize, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
                        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                        CLEARTYPE_QUALITY, FIXED_PITCH | FF_MODERN, L"Consolas");
                    HFONT oldF = static_cast<HFONT>(SelectObject(g_MeltDC, bigFont));
                    SetBkMode(g_MeltDC, TRANSPARENT);
                    SetTextColor(g_MeltDC, RGB(255, 0, 0));
                    int tx = g_MeltWidth / 2 - kHackedTextOffsetX;
                    int ty = g_MeltHeight / 2 - kHackedTextOffsetY;
                    TextOutW(g_MeltDC, tx, ty, L"YOU'VE BEEN HACKED", 18);
                    SelectObject(g_MeltDC, oldF);
                    DeleteObject(bigFont);
                }
            }

            // === PHASE 3: Fade to black ===
            if (3 == g_MeltPhase)
            {
                int progress = g_MeltTickCount - kMatrixEndTick;
                int numBlocks = progress * kFadeBlocksPerProgress;
                for (int i = 0; i < numBlocks && i < kFadeMaxBlocks; i++)
                {
                    int bx = rand() % g_MeltWidth;
                    int by = rand() % g_MeltHeight;
                    int bw = 10 + rand() % 80;
                    int bh = 5 + rand() % 40;
                    RECT r = { bx, by, bx + bw, by + bh };
                    HBRUSH black = CreateSolidBrush(RGB(0, 0, 0));
                    FillRect(g_MeltDC, &r, black);
                    DeleteObject(black);
                }
            }

            HDC hdcWnd = GetDC(hWnd);
            BitBlt(hdcWnd, 0, 0, g_MeltWidth, g_MeltHeight, g_MeltDC, 0, 0, SRCCOPY);
            ReleaseDC(hWnd, hdcWnd);

            return 0;
        }

        case WM_KEYDOWN:
            if (VK_ESCAPE == wParam)
            {
                KillTimer(hWnd, TIMER_MELT);
                DestroyWindow(hWnd);
            }
            return 0;

        case WM_LBUTTONDOWN:
        case WM_RBUTTONDOWN:
            KillTimer(hWnd, TIMER_MELT);
            DestroyWindow(hWnd);
            return 0;

        case WM_DESTROY:
            KillTimer(hWnd, TIMER_MELT);
            if (nullptr != g_MeltDC)
            {
                DeleteDC(g_MeltDC);
                g_MeltDC = NULL;
            }
            if (nullptr != g_MeltBitmap)
            {
                DeleteObject(g_MeltBitmap);
                g_MeltBitmap = NULL;
            }
            if (nullptr != g_MeltFont)
            {
                DeleteObject(g_MeltFont);
                g_MeltFont = NULL;
            }
            g_MeltWnd = NULL;
            return 0;
    }

    return DefWindowProcW(hWnd, msg, wParam, lParam);
}

// ============================================================================
// Spawn Screen Melt
// ============================================================================

void SpawnScreenMelt(const wchar_t* attackerName)
{
    if (nullptr != g_MeltWnd)
    {
        return;
    }

    if (nullptr != attackerName)
    {
        StringCchCopyW(g_MeltAttackerName, _countof(g_MeltAttackerName), attackerName);
    }
    else
    {
        StringCchCopyW(g_MeltAttackerName, _countof(g_MeltAttackerName), L"UNKNOWN");
    }

    int screenW = GetSystemMetrics(SM_CXSCREEN);
    int screenH = GetSystemMetrics(SM_CYSCREEN);

    g_MeltWnd = CreateWindowExW(
        WS_EX_TOPMOST,
        L"ShadowNetMeltClass",
        L"",
        WS_POPUP,
        0, 0, screenW, screenH,
        NULL,
        NULL,
        GetModuleHandle(NULL),
        NULL);

    if (nullptr != g_MeltWnd)
    {
        ShowWindow(g_MeltWnd, SW_SHOW);
        UpdateWindow(g_MeltWnd);
        SetForegroundWindow(g_MeltWnd);
    }
}

// ============================================================================
// Cleanup
// ============================================================================

void CleanupMeltWindow()
{
    if (nullptr != g_MeltWnd)
    {
        DestroyWindow(g_MeltWnd);
        g_MeltWnd = NULL;
    }
}
