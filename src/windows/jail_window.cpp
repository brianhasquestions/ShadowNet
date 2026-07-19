#include "jail_window.h"
#include "../core/game.h"
#include <strsafe.h>
#include <ctime>

// ============================================================================
// Jail Window State
// ============================================================================

HWND g_JailWnd = NULL;
static HFONT g_JailFontLarge = NULL;
static HFONT g_JailFontMedium = NULL;
static HFONT g_JailFontSmall = NULL;
static int g_JailTickCount = 0;
static int64_t g_JailEndTimestamp = 0;
static int g_JailScreenW = 0;
static int g_JailScreenH = 0;

// ============================================================================
// Jail Window Constants
// ============================================================================

constexpr int kLargeFontSize = 48;
constexpr int kMediumFontSize = 28;
constexpr int kSmallFontSize = 18;
constexpr int kBarDescentPadding = 40;
constexpr int kTextCenterOffsetY = 60;
constexpr int kCountdownOffsetY = 30;
constexpr int kSubtextOffsetY = 80;
constexpr int kBorderThickness = 8;
constexpr int kSecondsPerHour = 3600;
constexpr int kSecondsPerMinute = 60;
constexpr int kShadowOffset = 3;

// ============================================================================
// Jail Window Procedure
// ============================================================================

LRESULT CALLBACK JailWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
        case WM_CREATE:
        {
            g_JailScreenW = GetSystemMetrics(SM_CXSCREEN);
            g_JailScreenH = GetSystemMetrics(SM_CYSCREEN);
            g_JailTickCount = 0;

            g_JailFontLarge = CreateFontW(kLargeFontSize, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
                DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                CLEARTYPE_QUALITY, FIXED_PITCH | FF_MODERN, L"Consolas");

            g_JailFontMedium = CreateFontW(kMediumFontSize, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
                DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                CLEARTYPE_QUALITY, FIXED_PITCH | FF_MODERN, L"Consolas");

            g_JailFontSmall = CreateFontW(kSmallFontSize, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                CLEARTYPE_QUALITY, FIXED_PITCH | FF_MODERN, L"Consolas");

            SetTimer(hWnd, TIMER_JAIL, JAIL_TICK_MS, NULL);
            return 0;
        }

        case WM_TIMER:
        {
            if (TIMER_JAIL != wParam)
            {
                break;
            }

            g_JailTickCount++;
            InvalidateRect(hWnd, NULL, FALSE);
            return 0;
        }

        case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);

            RECT clientRc;
            GetClientRect(hWnd, &clientRc);
            int w = clientRc.right;
            int h = clientRc.bottom;

            // Black background
            HBRUSH bgBrush = CreateSolidBrush(RGB(0, 0, 0));
            FillRect(hdc, &clientRc, bgBrush);
            DeleteObject(bgBrush);

            // Calculate bar descent progress (0.0 to 1.0)
            float barProgress = 1.0f;
            if (g_JailTickCount < JAIL_DESCEND_DURATION_TICKS)
            {
                barProgress = static_cast<float>(g_JailTickCount) / static_cast<float>(JAIL_DESCEND_DURATION_TICKS);
            }

            // Draw jail bars descending from top
            int barSpacing = w / JAIL_BAR_COUNT;
            int barHeight = static_cast<int>(h * barProgress);
            HBRUSH barBrush = CreateSolidBrush(RGB(80, 80, 80));
            HBRUSH barHighlight = CreateSolidBrush(RGB(120, 120, 120));

            for (int i = 0; i < JAIL_BAR_COUNT; i++)
            {
                int barX = i * barSpacing + (barSpacing - JAIL_BAR_WIDTH) / 2;

                // Main bar
                RECT barRect = { barX, 0, barX + JAIL_BAR_WIDTH, barHeight };
                FillRect(hdc, &barRect, barBrush);

                // Highlight edge (3D effect)
                constexpr int kHighlightWidth = 3;
                RECT highlightRect = { barX, 0, barX + kHighlightWidth, barHeight };
                FillRect(hdc, &highlightRect, barHighlight);
            }

            // Horizontal crossbars (after bars fully descended)
            if (g_JailTickCount >= JAIL_DESCEND_DURATION_TICKS)
            {
                constexpr int kCrossbarCount = 4;
                constexpr int kCrossbarHeight = 10;
                for (int i = 1; i <= kCrossbarCount; i++)
                {
                    int crossY = (h * i) / (kCrossbarCount + 1);
                    RECT crossRect = { 0, crossY, w, crossY + kCrossbarHeight };
                    FillRect(hdc, &crossRect, barBrush);
                }
            }

            DeleteObject(barBrush);
            DeleteObject(barHighlight);

            // Police light flashing border (after bars are down)
            if (g_JailTickCount >= JAIL_DESCEND_DURATION_TICKS)
            {
                bool isRed = (0 == (g_JailTickCount / JAIL_POLICE_FLASH_INTERVAL) % 2);
                COLORREF flashColor = isRed ? RGB(255, 0, 0) : RGB(0, 0, 255);
                HBRUSH flashBrush = CreateSolidBrush(flashColor);

                RECT topBorder = { 0, 0, w, kBorderThickness };
                RECT bottomBorder = { 0, h - kBorderThickness, w, h };
                RECT leftBorder = { 0, 0, kBorderThickness, h };
                RECT rightBorder = { w - kBorderThickness, 0, w, h };

                FillRect(hdc, &topBorder, flashBrush);
                FillRect(hdc, &bottomBorder, flashBrush);
                FillRect(hdc, &leftBorder, flashBrush);
                FillRect(hdc, &rightBorder, flashBrush);

                DeleteObject(flashBrush);
            }

            SetBkMode(hdc, TRANSPARENT);

            // Main seizure text (after bars halfway down)
            if (g_JailTickCount > JAIL_DESCEND_DURATION_TICKS / 2)
            {
                HFONT oldFont = static_cast<HFONT>(SelectObject(hdc, g_JailFontLarge));

                const wchar_t* seizureText = L"THIS MACHINE HAS BEEN";
                const wchar_t* seizureText2 = L"SEIZED BY THE FBI";

                // Shadow
                SetTextColor(hdc, RGB(80, 0, 0));
                RECT shadowRc1 = { kShadowOffset, h / 2 - kTextCenterOffsetY + kShadowOffset, w + kShadowOffset, h };
                DrawTextW(hdc, seizureText, -1, &shadowRc1, DT_CENTER | DT_SINGLELINE);
                RECT shadowRc2 = { kShadowOffset, h / 2 - kTextCenterOffsetY + kLargeFontSize + kShadowOffset, w + kShadowOffset, h };
                DrawTextW(hdc, seizureText2, -1, &shadowRc2, DT_CENTER | DT_SINGLELINE);

                // Main text - pulsing red
                int pulse = 180 + static_cast<int>(75.0 * ((g_JailTickCount % 60) < 30 ? 1.0 : 0.6));
                if (pulse > 255)
                {
                    pulse = 255;
                }
                SetTextColor(hdc, RGB(pulse, 0, 0));
                RECT textRc1 = { 0, h / 2 - kTextCenterOffsetY, w, h };
                DrawTextW(hdc, seizureText, -1, &textRc1, DT_CENTER | DT_SINGLELINE);
                RECT textRc2 = { 0, h / 2 - kTextCenterOffsetY + kLargeFontSize, w, h };
                DrawTextW(hdc, seizureText2, -1, &textRc2, DT_CENTER | DT_SINGLELINE);

                SelectObject(hdc, oldFont);
            }

            // Countdown timer (after bars fully down)
            if (g_JailTickCount >= JAIL_DESCEND_DURATION_TICKS)
            {
                int64_t now = static_cast<int64_t>(time(NULL));
                int64_t remaining = g_JailEndTimestamp - now;
                if (remaining < 0)
                {
                    remaining = 0;
                }

                int hours = static_cast<int>(remaining / kSecondsPerHour);
                int minutes = static_cast<int>((remaining % kSecondsPerHour) / kSecondsPerMinute);
                int seconds = static_cast<int>(remaining % kSecondsPerMinute);

                HFONT oldFont = static_cast<HFONT>(SelectObject(hdc, g_JailFontMedium));
                SetTextColor(hdc, RGB(255, 255, 255));

                wchar_t countdownBuf[BUF_LARGE] = {};
                StringCchPrintfW(countdownBuf, BUF_LARGE, L"All operations suspended. Release in: %02d:%02d:%02d", hours, minutes, seconds);

                RECT countdownRc = { 0, h / 2 + kCountdownOffsetY, w, h };
                DrawTextW(hdc, countdownBuf, -1, &countdownRc, DT_CENTER | DT_SINGLELINE);

                SelectObject(hdc, oldFont);

                // Subtext
                HFONT oldFont2 = static_cast<HFONT>(SelectObject(hdc, g_JailFontSmall));
                SetTextColor(hdc, RGB(150, 150, 150));

                RECT subtextRc = { 0, h / 2 + kSubtextOffsetY + kCountdownOffsetY, w, h };
                DrawTextW(hdc, L"Press ESC or click to dismiss this window. Jail persists.", -1, &subtextRc, DT_CENTER | DT_SINGLELINE);

                SelectObject(hdc, oldFont2);
            }

            EndPaint(hWnd, &ps);
            return 0;
        }

        case WM_ERASEBKGND:
            return 1;

        case WM_KEYDOWN:
            if (VK_ESCAPE == wParam)
            {
                KillTimer(hWnd, TIMER_JAIL);
                DestroyWindow(hWnd);
            }
            return 0;

        case WM_LBUTTONDOWN:
        case WM_RBUTTONDOWN:
            KillTimer(hWnd, TIMER_JAIL);
            DestroyWindow(hWnd);
            return 0;

        case WM_DESTROY:
            KillTimer(hWnd, TIMER_JAIL);
            if (nullptr != g_JailFontLarge)
            {
                DeleteObject(g_JailFontLarge);
                g_JailFontLarge = NULL;
            }
            if (nullptr != g_JailFontMedium)
            {
                DeleteObject(g_JailFontMedium);
                g_JailFontMedium = NULL;
            }
            if (nullptr != g_JailFontSmall)
            {
                DeleteObject(g_JailFontSmall);
                g_JailFontSmall = NULL;
            }
            g_JailWnd = NULL;
            return 0;
    }

    return DefWindowProcW(hWnd, msg, wParam, lParam);
}

// ============================================================================
// Spawn Jail Window
// ============================================================================

void SpawnJailWindow(int64_t jailEndTimestamp)
{
    if (nullptr != g_JailWnd)
    {
        return;
    }

    g_JailEndTimestamp = jailEndTimestamp;

    int screenW = GetSystemMetrics(SM_CXSCREEN);
    int screenH = GetSystemMetrics(SM_CYSCREEN);

    g_JailWnd = CreateWindowExW(
        WS_EX_TOPMOST,
        L"ShadowNetJailClass",
        L"",
        WS_POPUP,
        0, 0, screenW, screenH,
        NULL,
        NULL,
        GetModuleHandle(NULL),
        NULL);

    if (nullptr != g_JailWnd)
    {
        ShowWindow(g_JailWnd, SW_SHOW);
        UpdateWindow(g_JailWnd);
        SetForegroundWindow(g_JailWnd);
    }
}

// ============================================================================
// Cleanup
// ============================================================================

void CleanupJailWindow()
{
    if (nullptr != g_JailWnd)
    {
        DestroyWindow(g_JailWnd);
        g_JailWnd = NULL;
    }
}
