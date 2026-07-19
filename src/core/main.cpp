#ifndef UNICODE
#define UNICODE
#endif
#ifndef _UNICODE
#define _UNICODE
#endif

#include <windows.h>
#include <dwmapi.h>
#include <shobjidl.h>
#include <strsafe.h>
#include <memory>
#include <cstdio>
#include <cstring>
#include <ctime>

#include "game.h"
#include "../resource.h"
#include "../ui/colors.h"
#include "../ui/ui.h"
#include "../data/strings.h"
#include "../windows/irc_window.h"
#include "../windows/boss_window.h"
#include "../windows/melt_window.h"
#include "../windows/jail_window.h"
#include "../systems/actions.h"
#include "../systems/skills.h"

// ============================================================================
// Globals
// ============================================================================

static GameState g_GameState;
static UIFonts g_Fonts;
static HWND g_hWnd = NULL;

// Spider animation
static HWND g_SpiderWnd = NULL;
static int g_SpiderX = 0;
static int g_SpiderY = 0;           // spider's Y position on screen (top of spider body)
static int g_SpiderTargetY = 0;
static int g_SpiderPhase = 0;       // 0=dropping, 1=pause, 2=descend to taskbar, 3=walking, 4=done
static int g_SpiderFrame = 0;
static int g_SpiderPauseTimer = 0;
static bool g_SpiderSideView = false;
static int g_SpiderSpawnDelay = 0;    // ticks until spider spawns (overlay timer ticks)
static bool g_SpiderSpawnPending = false;
static constexpr int kSpiderSize = 16;       // sprite is 16x16
static constexpr int kSpiderScale = 2;       // drawn at 2x
static constexpr int kSpiderDrawSize = kSpiderSize * kSpiderScale; // 32px
static constexpr int kSpiderDropSpeed = 5;
static constexpr int kSpiderWalkSpeed = 2;
static constexpr int kSpiderPauseTicks = 20;
static constexpr UINT_PTR TIMER_SPIDER = 11;
static constexpr int kSpiderTickMs = 30;
static constexpr int kSpiderLegCycleRate = 3; // change legs every N ticks

// Mini overlay (shown when minimized)
static HWND g_OverlayWnd = NULL;
static constexpr int kOverlayWidth = 220;
static constexpr int kOverlayHeight = 60;
static constexpr UINT_PTR TIMER_OVERLAY = 10;
static constexpr int kOverlayRenderMs = 250;

// Taskbar progress
static ITaskbarList3* g_pTaskbar = nullptr;

// Double buffering
static HDC g_MemDC = NULL;
static HBITMAP g_MemBitmap = NULL;
static int g_BufWidth = 0;
static int g_BufHeight = 0;

// Timer IDs
constexpr UINT_PTR TIMER_GAME_TICK = 1;
constexpr UINT_PTR TIMER_RENDER = 2;
constexpr UINT_PTR TIMER_TOAST = 3;

// Timer intervals (ms)
constexpr int kGameTickIntervalMs = 1000;
constexpr int kRenderIntervalMs = 100;
constexpr int kToastIntervalMs = 100;

// Window dimensions
constexpr int kDefaultWindowWidth = 1280;
constexpr int kDefaultWindowHeight = 800;
constexpr int kMinWindowWidth = 1024;
constexpr int kMinWindowHeight = 768;

// Scroll
constexpr int kScrollAmount = 3;

// ============================================================================
// Double Buffer Management
// ============================================================================

static void EnsureBackBuffer(HDC hdc, int width, int height)
{
    if (nullptr != g_MemDC && g_BufWidth == width && g_BufHeight == height)
    {
        return;
    }

    if (nullptr != g_MemBitmap)
    {
        DeleteObject(g_MemBitmap);
    }
    if (nullptr != g_MemDC)
    {
        DeleteDC(g_MemDC);
    }

    g_MemDC = CreateCompatibleDC(hdc);
    g_MemBitmap = CreateCompatibleBitmap(hdc, width, height);
    SelectObject(g_MemDC, g_MemBitmap);
    g_BufWidth = width;
    g_BufHeight = height;
}

// ============================================================================
// Rendering
// ============================================================================

static void RenderFrame(HWND hWnd)
{
    RECT rc;
    GetClientRect(hWnd, &rc);
    int w = rc.right - rc.left;
    int h = rc.bottom - rc.top;
    if (w <= 0 || h <= 0)
    {
        return;
    }

    HDC hdc = GetDC(hWnd);
    EnsureBackBuffer(hdc, w, h);

    DrawFilledRect(g_MemDC, RECT{ 0, 0, w, h }, Colors::BackgroundPrimary);

    DrawSidebar(DrawCtx{ g_MemDC, &g_Fonts }, &g_GameState, h);

    DrawStatusBar(DrawCtx{ g_MemDC, &g_Fonts }, &g_GameState, WindowSize{ w, h });

    int panelX = SIDEBAR_WIDTH;
    int panelY = 0;
    int panelW = w - SIDEBAR_WIDTH;
    int panelH = h - STATUSBAR_HEIGHT;

    switch (g_GameState.currentTab)
    {
        case Tab::Dashboard:
            DrawDashboardPanel(DrawCtx{ g_MemDC, &g_Fonts }, &g_GameState, PanelRect{ panelX, panelY, panelW, panelH });
            break;
        case Tab::Jobs:
            DrawJobsPanel(DrawCtx{ g_MemDC, &g_Fonts }, &g_GameState, PanelRect{ panelX, panelY, panelW, panelH });
            break;
        case Tab::Personnel:
            DrawPersonnelPanel(DrawCtx{ g_MemDC, &g_Fonts }, &g_GameState, PanelRect{ panelX, panelY, panelW, panelH });
            break;
        case Tab::Skills:
            DrawSkillsPanel(DrawCtx{ g_MemDC, &g_Fonts }, &g_GameState, PanelRect{ panelX, panelY, panelW, panelH });
            break;
        case Tab::Market:
            DrawMarketPanel(DrawCtx{ g_MemDC, &g_Fonts }, &g_GameState, PanelRect{ panelX, panelY, panelW, panelH });
            break;
        case Tab::Intel:
            DrawIntelPanel(DrawCtx{ g_MemDC, &g_Fonts }, &g_GameState, PanelRect{ panelX, panelY, panelW, panelH });
            break;
        case Tab::Military:
            DrawMilitaryPanel(DrawCtx{ g_MemDC, &g_Fonts }, &g_GameState, PanelRect{ panelX, panelY, panelW, panelH });
            break;
        case Tab::Prestige:
            DrawPrestigePanel(DrawCtx{ g_MemDC, &g_Fonts }, &g_GameState, PanelRect{ panelX, panelY, panelW, panelH });
            break;
        default:
            break;
    }

    DrawToasts(DrawCtx{ g_MemDC, &g_Fonts }, &g_GameState, WindowSize{ w, h });

    BitBlt(hdc, 0, 0, w, h, g_MemDC, 0, 0, SRCCOPY);
    ReleaseDC(hWnd, hdc);
}

// ============================================================================
// Spider Animation
// ============================================================================

// 16x16 spider sprite - 4 walk frames for scurrying legs
// 0=transparent, 1=body, 2=leg, 3=eye(red)
static const uint8_t kSpiderFrames[4][kSpiderSize][kSpiderSize] = {
    // Frame 0: legs out wide
    {
        {0,0,2,0,0,0,0,0,0,0,0,0,0,2,0,0},
        {0,0,0,2,0,0,0,0,0,0,0,0,2,0,0,0},
        {2,0,0,0,2,0,0,1,1,0,0,2,0,0,0,2},
        {0,2,0,0,0,2,1,1,1,1,2,0,0,0,2,0},
        {0,0,2,0,0,1,1,3,3,1,1,0,0,2,0,0},
        {0,0,0,2,1,1,1,1,1,1,1,1,2,0,0,0},
        {0,0,0,0,1,1,1,1,1,1,1,1,0,0,0,0},
        {0,0,0,2,1,1,1,1,1,1,1,1,2,0,0,0},
        {0,0,2,0,0,1,1,1,1,1,1,0,0,2,0,0},
        {0,2,0,0,0,0,1,1,1,1,0,0,0,0,2,0},
        {2,0,0,0,0,0,0,1,1,0,0,0,0,0,0,2},
        {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    },
    // Frame 1: legs mid-up
    {
        {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
        {0,0,2,0,0,0,0,0,0,0,0,0,0,2,0,0},
        {0,2,0,2,0,0,0,1,1,0,0,0,2,0,2,0},
        {0,0,0,0,2,0,1,1,1,1,0,2,0,0,0,0},
        {0,0,0,2,0,1,1,3,3,1,1,0,2,0,0,0},
        {0,0,2,0,1,1,1,1,1,1,1,1,0,2,0,0},
        {0,0,0,0,1,1,1,1,1,1,1,1,0,0,0,0},
        {0,0,2,0,1,1,1,1,1,1,1,1,0,2,0,0},
        {0,0,0,2,0,1,1,1,1,1,1,0,2,0,0,0},
        {0,0,0,0,2,0,1,1,1,1,0,2,0,0,0,0},
        {0,0,0,2,0,0,0,1,1,0,0,0,2,0,0,0},
        {0,0,2,0,0,0,0,0,0,0,0,0,0,2,0,0},
        {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    },
    // Frame 2: legs tucked in
    {
        {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
        {0,0,0,2,0,0,0,1,1,0,0,0,2,0,0,0},
        {0,0,0,0,2,0,1,1,1,1,0,2,0,0,0,0},
        {0,0,0,0,2,1,1,3,3,1,1,2,0,0,0,0},
        {0,0,0,2,1,1,1,1,1,1,1,1,2,0,0,0},
        {0,0,0,0,1,1,1,1,1,1,1,1,0,0,0,0},
        {0,0,0,2,1,1,1,1,1,1,1,1,2,0,0,0},
        {0,0,0,0,2,1,1,1,1,1,1,2,0,0,0,0},
        {0,0,0,0,2,0,1,1,1,1,0,2,0,0,0,0},
        {0,0,0,2,0,0,0,1,1,0,0,0,2,0,0,0},
        {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    },
    // Frame 3: legs mid-down (mirrors frame 1 but legs down)
    {
        {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
        {0,0,0,2,0,0,0,0,0,0,0,0,2,0,0,0},
        {0,0,2,0,0,0,0,1,1,0,0,0,0,2,0,0},
        {0,0,0,0,2,0,1,1,1,1,0,2,0,0,0,0},
        {0,0,0,0,0,1,1,3,3,1,1,0,0,0,0,0},
        {0,0,0,2,1,1,1,1,1,1,1,1,2,0,0,0},
        {0,0,0,0,1,1,1,1,1,1,1,1,0,0,0,0},
        {0,0,0,2,1,1,1,1,1,1,1,1,2,0,0,0},
        {0,0,0,0,0,1,1,1,1,1,1,0,0,0,0,0},
        {0,0,0,0,2,0,1,1,1,1,0,2,0,0,0,0},
        {0,0,0,2,0,0,0,1,1,0,0,0,2,0,0,0},
        {0,0,2,0,0,0,0,0,0,0,0,0,0,2,0,0},
        {0,2,0,0,0,0,0,0,0,0,0,0,0,0,2,0},
        {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    },
};

// Side-view spider walking left - 4 frames with scurrying legs
// Viewed from the side: body is round, legs extend down, eyes on left
static const uint8_t kSpiderSideFrames[4][kSpiderSize][kSpiderSize] = {
    // Frame 0: legs stride A
    {
        {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,1,1,1,1,1,1,0,0,0,0},
        {0,0,0,0,0,1,1,1,1,1,1,1,1,0,0,0},
        {0,0,0,0,1,1,1,1,1,1,1,1,1,1,0,0},
        {0,0,0,3,1,1,1,1,1,1,1,1,1,1,0,0},
        {0,0,0,3,1,1,1,1,1,1,1,1,1,1,0,0},
        {0,0,0,0,1,1,1,1,1,1,1,1,1,1,0,0},
        {0,0,0,0,0,1,1,1,1,1,1,1,1,0,0,0},
        {0,0,0,0,0,2,2,1,2,1,2,1,2,0,0,0},
        {0,0,0,0,2,0,0,2,0,2,0,2,0,2,0,0},
        {0,0,0,2,0,0,2,0,0,0,2,0,0,0,2,0},
        {0,0,2,0,0,2,0,0,0,0,0,0,0,0,0,2},
        {0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    },
    // Frame 1: legs stride B
    {
        {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,1,1,1,1,1,1,0,0,0,0},
        {0,0,0,0,0,1,1,1,1,1,1,1,1,0,0,0},
        {0,0,0,0,1,1,1,1,1,1,1,1,1,1,0,0},
        {0,0,0,3,1,1,1,1,1,1,1,1,1,1,0,0},
        {0,0,0,3,1,1,1,1,1,1,1,1,1,1,0,0},
        {0,0,0,0,1,1,1,1,1,1,1,1,1,1,0,0},
        {0,0,0,0,0,1,1,1,1,1,1,1,1,0,0,0},
        {0,0,0,0,0,2,1,2,1,2,1,2,2,0,0,0},
        {0,0,0,0,0,0,2,0,2,0,2,0,0,0,0,0},
        {0,0,0,0,0,2,0,0,0,2,0,0,2,0,0,0},
        {0,0,0,0,2,0,0,0,2,0,0,0,0,2,0,0},
        {0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    },
    // Frame 2: legs stride C
    {
        {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,1,1,1,1,1,1,0,0,0,0},
        {0,0,0,0,0,1,1,1,1,1,1,1,1,0,0,0},
        {0,0,0,0,1,1,1,1,1,1,1,1,1,1,0,0},
        {0,0,0,3,1,1,1,1,1,1,1,1,1,1,0,0},
        {0,0,0,3,1,1,1,1,1,1,1,1,1,1,0,0},
        {0,0,0,0,1,1,1,1,1,1,1,1,1,1,0,0},
        {0,0,0,0,0,1,1,1,1,1,1,1,1,0,0,0},
        {0,0,0,0,0,2,2,1,2,1,2,1,2,0,0,0},
        {0,0,0,0,0,0,0,2,0,2,0,2,0,0,0,0},
        {0,0,0,0,0,0,2,0,0,0,2,0,0,2,0,0},
        {0,0,0,0,0,2,0,0,0,0,0,0,2,0,0,0},
        {0,0,0,0,2,0,0,0,0,0,0,0,0,0,2,0},
        {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    },
    // Frame 3: legs stride D
    {
        {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,1,1,1,1,1,1,0,0,0,0},
        {0,0,0,0,0,1,1,1,1,1,1,1,1,0,0,0},
        {0,0,0,0,1,1,1,1,1,1,1,1,1,1,0,0},
        {0,0,0,3,1,1,1,1,1,1,1,1,1,1,0,0},
        {0,0,0,3,1,1,1,1,1,1,1,1,1,1,0,0},
        {0,0,0,0,1,1,1,1,1,1,1,1,1,1,0,0},
        {0,0,0,0,0,1,1,1,1,1,1,1,1,0,0,0},
        {0,0,0,0,0,2,1,2,1,2,1,2,2,0,0,0},
        {0,0,0,0,2,0,2,0,2,0,2,0,0,2,0,0},
        {0,0,0,2,0,2,0,0,0,2,0,0,0,0,2,0},
        {0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    },
};

static void UpdateSpiderBitmap()
{
    if (nullptr == g_SpiderWnd)
    {
        return;
    }

    int animFrame = (g_SpiderFrame / kSpiderLegCycleRate) % 4;

    // During drop/pause/descend phases, window spans from y=0 to spider bottom (for the web line)
    // During walk phase, window is just the spider
    int bmpW = kSpiderDrawSize;
    int bmpH = 0;
    int winX = g_SpiderX;
    int winY = 0;
    int spiderDrawY = 0; // where in the bitmap to draw the spider

    if (0 == g_SpiderPhase || 1 == g_SpiderPhase || 2 == g_SpiderPhase)
    {
        // Window from top of screen to bottom of spider
        winY = 0;
        bmpH = g_SpiderY + kSpiderDrawSize;
        if (bmpH < kSpiderDrawSize) bmpH = kSpiderDrawSize;
        spiderDrawY = g_SpiderY;
    }
    else
    {
        // Walking: window is just the spider
        winY = g_SpiderY;
        bmpH = kSpiderDrawSize;
        spiderDrawY = 0;
    }

    if (bmpH <= 0 || bmpW <= 0)
    {
        return;
    }

    BITMAPINFO bmi = {};
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = bmpW;
    bmi.bmiHeader.biHeight = -bmpH; // top-down
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;

    uint32_t* pixels = nullptr;
    HDC screenDC = GetDC(NULL);
    HDC memDC = CreateCompatibleDC(screenDC);
    HBITMAP hBmp = CreateDIBSection(memDC, &bmi, DIB_RGB_COLORS, reinterpret_cast<void**>(&pixels), NULL, 0);
    HBITMAP oldBmp = static_cast<HBITMAP>(SelectObject(memDC, hBmp));

    ZeroMemory(pixels, bmpW * bmpH * 4);

    // Draw silk thread (white line from top of window to top of spider)
    if (0 == g_SpiderPhase || 1 == g_SpiderPhase || 2 == g_SpiderPhase)
    {
        int threadX = bmpW / 2;
        for (int ty = 0; ty < spiderDrawY; ty++)
        {
            if (ty >= 0 && ty < bmpH)
            {
                pixels[ty * bmpW + threadX] = 0xFFDDDDDD;
                // slight width variation for natural look
                if ((ty % 8) < 4 && threadX + 1 < bmpW)
                {
                    pixels[ty * bmpW + threadX + 1] = 0x80AAAAAA;
                }
            }
        }
    }

    // Draw spider at 2x scale - pick top-down or side view
    const uint8_t (*spriteSet)[kSpiderSize][kSpiderSize] = g_SpiderSideView ? kSpiderSideFrames : kSpiderFrames;

    for (int sy = 0; sy < kSpiderSize; sy++)
    {
        for (int sx = 0; sx < kSpiderSize; sx++)
        {
            uint8_t val = spriteSet[animFrame][sy][sx];
            if (0 == val) continue;

            uint32_t color = 0;
            if (1 == val) color = 0xFF1A1A2E;
            else if (2 == val) color = 0xFF3A3A5A;
            else if (3 == val) color = 0xFFFF2222;

            for (int dy = 0; dy < kSpiderScale; dy++)
            {
                for (int dx = 0; dx < kSpiderScale; dx++)
                {
                    int px = sx * kSpiderScale + dx;
                    int py = spiderDrawY + sy * kSpiderScale + dy;
                    if (px < bmpW && py >= 0 && py < bmpH)
                    {
                        pixels[py * bmpW + px] = color;
                    }
                }
            }
        }
    }

    POINT ptSrc = {0, 0};
    SIZE sizeWnd = {bmpW, bmpH};
    BLENDFUNCTION blend = {};
    blend.BlendOp = AC_SRC_OVER;
    blend.SourceConstantAlpha = 255;
    blend.AlphaFormat = AC_SRC_ALPHA;

    POINT ptPos = {winX, winY};
    UpdateLayeredWindow(g_SpiderWnd, screenDC, &ptPos, &sizeWnd, memDC, &ptSrc, 0, &blend, ULW_ALPHA);

    SelectObject(memDC, oldBmp);
    DeleteObject(hBmp);
    DeleteDC(memDC);
    ReleaseDC(NULL, screenDC);
}

static LRESULT CALLBACK SpiderWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
        case WM_TIMER:
        {
            g_SpiderFrame++;

            if (0 == g_SpiderPhase)
            {
                // Dropping down on silk thread (top-down view)
                g_SpiderY += kSpiderDropSpeed;
                if (g_SpiderY >= g_SpiderTargetY)
                {
                    g_SpiderY = g_SpiderTargetY;
                    g_SpiderPhase = 1;
                    g_SpiderPauseTimer = 0;
                }
            }
            else if (1 == g_SpiderPhase)
            {
                // Pause on overlay
                g_SpiderPauseTimer++;
                if (g_SpiderPauseTimer >= kSpiderPauseTicks)
                {
                    // Start descending to taskbar
                    g_SpiderPhase = 2;
                    RECT taskbarRect;
                    HWND hTaskbar = FindWindowW(L"Shell_TrayWnd", NULL);
                    if (nullptr != hTaskbar && GetWindowRect(hTaskbar, &taskbarRect))
                    {
                        g_SpiderTargetY = taskbarRect.top - kSpiderDrawSize;
                    }
                    else
                    {
                        int screenH = GetSystemMetrics(SM_CYSCREEN);
                        g_SpiderTargetY = screenH - 48 - kSpiderDrawSize;
                    }
                }
            }
            else if (2 == g_SpiderPhase)
            {
                // Descending from overlay to taskbar on silk thread
                g_SpiderY += kSpiderDropSpeed;
                if (g_SpiderY >= g_SpiderTargetY)
                {
                    g_SpiderY = g_SpiderTargetY;
                    g_SpiderPhase = 3;
                    g_SpiderSideView = true; // switch to side view for walking
                }
            }
            else if (3 == g_SpiderPhase)
            {
                // Scurrying left across top of taskbar (side view)
                g_SpiderX -= kSpiderWalkSpeed;

                if (g_SpiderX < -kSpiderDrawSize)
                {
                    g_SpiderPhase = 4;
                    KillTimer(hWnd, TIMER_SPIDER);
                    ShowWindow(hWnd, SW_HIDE);
                    return 0;
                }
            }

            UpdateSpiderBitmap();
            return 0;
        }

        case WM_ERASEBKGND:
            return 1;
    }
    return DefWindowProcW(hWnd, msg, wParam, lParam);
}

static void SpawnSpider()
{
    if (nullptr == g_SpiderWnd)
    {
        return;
    }

    // Drop onto the overlay from the top of the screen
    int screenW = GetSystemMetrics(SM_CXSCREEN);
    int screenH = GetSystemMetrics(SM_CYSCREEN);

    // Start above the overlay, centered on it
    g_SpiderX = screenW - kOverlayWidth - 12 + kOverlayWidth / 2 - kSpiderDrawSize / 2;
    g_SpiderY = 0;
    g_SpiderTargetY = screenH - kOverlayHeight - 50 - kSpiderDrawSize + 4; // land on top of overlay
    g_SpiderPhase = 0;
    g_SpiderFrame = 0;
    g_SpiderPauseTimer = 0;
    g_SpiderSideView = false;

    ShowWindow(g_SpiderWnd, SW_SHOWNOACTIVATE);
    UpdateSpiderBitmap();
    SetTimer(g_SpiderWnd, TIMER_SPIDER, kSpiderTickMs, NULL);
}

static void DespawnSpider()
{
    if (nullptr == g_SpiderWnd)
    {
        return;
    }
    KillTimer(g_SpiderWnd, TIMER_SPIDER);
    ShowWindow(g_SpiderWnd, SW_HIDE);
    g_SpiderPhase = 4;
}

// ============================================================================
// Mini Overlay (shown when minimized)
// ============================================================================

static void UpdateTaskbarProgress()
{
    if (nullptr == g_pTaskbar)
    {
        return;
    }

    int activeCount = GetActiveJobCount(&g_GameState);
    if (activeCount > 0)
    {
        float totalProgress = 0.0f;
        for (int i = 0; i < MAX_ACTIVE_JOBS; i++)
        {
            if (false == g_GameState.activeJobs[i].active)
            {
                continue;
            }
            const ActiveJob& aj = g_GameState.activeJobs[i];
            float p = 1.0f - (aj.remainingSeconds / static_cast<float>(aj.totalSeconds));
            if (p < 0.0f) p = 0.0f;
            if (p > 1.0f) p = 1.0f;
            totalProgress += p;
        }
        float avg = totalProgress / static_cast<float>(activeCount);
        g_pTaskbar->SetProgressValue(g_hWnd, static_cast<ULONGLONG>(avg * 1000), 1000);
        g_pTaskbar->SetProgressState(g_hWnd, TBPF_NORMAL);
    }
    else
    {
        g_pTaskbar->SetProgressState(g_hWnd, TBPF_NOPROGRESS);
    }
}

static void RenderOverlay(HWND hWnd)
{
    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(hWnd, &ps);

    // Background
    RECT rc = {0, 0, kOverlayWidth, kOverlayHeight};
    HBRUSH bgBrush = CreateSolidBrush(Colors::BackgroundPrimary);
    FillRect(hdc, &rc, bgBrush);
    DeleteObject(bgBrush);

    // Border
    HPEN pen = CreatePen(PS_SOLID, 1, Colors::Comment);
    HPEN oldPen = static_cast<HPEN>(SelectObject(hdc, pen));
    HBRUSH oldBrush = static_cast<HBRUSH>(SelectObject(hdc, GetStockObject(NULL_BRUSH)));
    Rectangle(hdc, 0, 0, kOverlayWidth, kOverlayHeight);
    SelectObject(hdc, oldBrush);
    SelectObject(hdc, oldPen);
    DeleteObject(pen);

    SetBkMode(hdc, TRANSPARENT);

    constexpr int kBarX = 50;
    constexpr int kBarWidth = 160;
    constexpr int kBarHeight = 10;
    constexpr int kLabelX = 6;

    // Jobs progress bar
    int activeCount = GetActiveJobCount(&g_GameState);
    HFONT font = CreateFontW(13, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
        OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, FIXED_PITCH, L"Consolas");
    HFONT oldFont = static_cast<HFONT>(SelectObject(hdc, font));

    SetTextColor(hdc, Colors::Cyan);
    TextOutW(hdc, kLabelX, 7, L"Jobs", 4);

    // Jobs bar background
    RECT barBg = {kBarX, 9, kBarX + kBarWidth, 9 + kBarHeight};
    HBRUSH barBgBrush = CreateSolidBrush(Colors::BackgroundSecondary);
    FillRect(hdc, &barBg, barBgBrush);
    DeleteObject(barBgBrush);

    if (activeCount > 0)
    {
        float totalProgress = 0.0f;
        for (int i = 0; i < MAX_ACTIVE_JOBS; i++)
        {
            if (false == g_GameState.activeJobs[i].active)
            {
                continue;
            }
            const ActiveJob& aj = g_GameState.activeJobs[i];
            float p = 1.0f - (aj.remainingSeconds / static_cast<float>(aj.totalSeconds));
            if (p < 0.0f) p = 0.0f;
            if (p > 1.0f) p = 1.0f;
            totalProgress += p;
        }
        float avg = totalProgress / static_cast<float>(activeCount);
        int fillW = static_cast<int>(kBarWidth * avg);
        RECT barFill = {kBarX, 9, kBarX + fillW, 9 + kBarHeight};
        HBRUSH fillBrush = CreateSolidBrush(Colors::Cyan);
        FillRect(hdc, &barFill, fillBrush);
        DeleteObject(fillBrush);
    }

    // Skill progress bar
    SetTextColor(hdc, Colors::Purple);
    TextOutW(hdc, kLabelX, 32, L"Skill", 5);

    RECT skillBg = {kBarX, 34, kBarX + kBarWidth, 34 + kBarHeight};
    HBRUSH skillBgBrush = CreateSolidBrush(Colors::BackgroundSecondary);
    FillRect(hdc, &skillBg, skillBgBrush);
    DeleteObject(skillBgBrush);

    if (true == g_GameState.docStudyActive)
    {
        float studyProgress = 1.0f - (g_GameState.docStudyRemaining / static_cast<float>(g_GameState.docStudyTotal));
        if (studyProgress < 0.0f) studyProgress = 0.0f;
        if (studyProgress > 1.0f) studyProgress = 1.0f;
        int fillW = static_cast<int>(kBarWidth * studyProgress);
        RECT skillFill = {kBarX, 34, kBarX + fillW, 34 + kBarHeight};
        HBRUSH fillBrush = CreateSolidBrush(Colors::Purple);
        FillRect(hdc, &skillFill, fillBrush);
        DeleteObject(fillBrush);
    }

    SelectObject(hdc, oldFont);
    DeleteObject(font);

    EndPaint(hWnd, &ps);
}

static LRESULT CALLBACK OverlayWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
        case WM_PAINT:
            RenderOverlay(hWnd);
            return 0;

        case WM_TIMER:
            InvalidateRect(hWnd, NULL, FALSE);
            UpdateTaskbarProgress();

            // Spider spawn countdown
            if (true == g_SpiderSpawnPending)
            {
                g_SpiderSpawnDelay--;
                if (g_SpiderSpawnDelay <= 0)
                {
                    g_SpiderSpawnPending = false;
                    SpawnSpider();
                }
            }
            return 0;

        case WM_LBUTTONDOWN:
            // Restore main window
            ShowWindow(g_hWnd, SW_RESTORE);
            SetForegroundWindow(g_hWnd);
            return 0;

        case WM_ERASEBKGND:
            return 1;
    }
    return DefWindowProcW(hWnd, msg, wParam, lParam);
}

static void ShowOverlay()
{
    if (nullptr == g_OverlayWnd)
    {
        return;
    }
    // Position in bottom-right corner of the screen
    int screenW = GetSystemMetrics(SM_CXSCREEN);
    int screenH = GetSystemMetrics(SM_CYSCREEN);
    int x = screenW - kOverlayWidth - 12;
    int y = screenH - kOverlayHeight - 50; // above taskbar

    SetWindowPos(g_OverlayWnd, HWND_TOPMOST, x, y, kOverlayWidth, kOverlayHeight, SWP_SHOWWINDOW);
    SetTimer(g_OverlayWnd, TIMER_OVERLAY, kOverlayRenderMs, NULL);
}

static void HideOverlay()
{
    if (nullptr == g_OverlayWnd)
    {
        return;
    }
    KillTimer(g_OverlayWnd, TIMER_OVERLAY);
    ShowWindow(g_OverlayWnd, SW_HIDE);
}

// ============================================================================
// Window Procedure
// ============================================================================

static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
        case WM_CREATE:
            CreateUIFonts(&g_Fonts);

            InitGameState(&g_GameState);
            if (false == LoadGame(&g_GameState))
            {
                // Fresh game, already initialized
            }

            SetTimer(hWnd, TIMER_GAME_TICK, kGameTickIntervalMs, NULL);
            SetTimer(hWnd, TIMER_RENDER, kRenderIntervalMs, NULL);
            SetTimer(hWnd, TIMER_TOAST, kToastIntervalMs, NULL);
            return 0;

        case WM_TIMER:
            if (TIMER_GAME_TICK == wParam)
            {
                GameTick(&g_GameState);

                if (g_GameState.pendingJobUnlockId >= 0 && nullptr == g_IRCWnd)
                {
                    SpawnJobUnlockIRCWindow(&g_GameState, g_hWnd, g_GameState.pendingJobUnlockId);
                    g_GameState.pendingJobUnlockId = -1;
                }

                if (true == g_GameState.triggerScreenMelt && nullptr == g_MeltWnd)
                {
                    SpawnScreenMelt(g_GameState.meltAttackerName);
                    g_GameState.triggerScreenMelt = false;
                }

                // Check for jail trigger (fed seizure)
                if (true == g_GameState.triggerJail && nullptr == g_JailWnd)
                {
                    SpawnJailWindow(g_GameState.jailEndTimestamp);
                    g_GameState.triggerJail = false;
                }
            }
            else if (TIMER_RENDER == wParam)
            {
                RenderFrame(hWnd);
                UpdateTaskbarProgress();
            }
            else if (TIMER_TOAST == wParam)
            {
                for (int i = 0; i < MAX_TOASTS; i++)
                {
                    if (true == g_GameState.toasts[i].active)
                    {
                        g_GameState.toasts[i].remainingMs -= kToastIntervalMs;
                        if (g_GameState.toasts[i].remainingMs <= 0)
                        {
                            g_GameState.toasts[i].active = false;
                        }
                    }
                }
            }
            return 0;

        case WM_PAINT:
        {
            PAINTSTRUCT ps;
            BeginPaint(hWnd, &ps);
            RenderFrame(hWnd);
            EndPaint(hWnd, &ps);
            return 0;
        }

        case WM_SIZE:
            if (SIZE_MINIMIZED == wParam)
            {
                ShowOverlay();
                // Schedule spider to appear at a random time within 5 minutes
                // Overlay timer ticks at kOverlayRenderMs (250ms), so 5 min = 1200 ticks
                g_SpiderSpawnDelay = GameRandomRange(&g_GameState, 1, 1200);
                g_SpiderSpawnPending = true;
            }
            else
            {
                HideOverlay();
                DespawnSpider();
                g_SpiderSpawnPending = false;
            }
            InvalidateRect(hWnd, NULL, FALSE);
            return 0;

        case WM_LBUTTONDOWN:
        {
            int mx = LOWORD(lParam);
            int my = HIWORD(lParam);

            RECT rc;
            GetClientRect(hWnd, &rc);
            int winW = rc.right - rc.left;
            int winH = rc.bottom - rc.top;

            HitResult hit = HitTest(&g_GameState, POINT{ mx, my }, WindowSize{ winW, winH });

            switch (hit.type)
            {
                case HitResult::SidebarTab:
                    g_GameState.currentTab = static_cast<Tab>(hit.index);
                    break;

                case HitResult::JobStart:
                {
                    // Auto-assign an idle crew member (specialist preferred) to
                    // help with the job; -1 runs it manually.
                    int pid = FindIdlePersonnelForJob(&g_GameState, hit.index);
                    StartJob(&g_GameState, hit.index, pid);
                    break;
                }

                case HitResult::JobFilter:
                    g_GameState.jobFilterTier = hit.index;
                    g_GameState.jobScrollOffset = 0;
                    break;

                case HitResult::PersonnelHire:
                    HirePersonnel(&g_GameState, hit.index, SpawnIRCWindow, g_hWnd, SpawnScreenMelt);
                    break;

                case HitResult::PersonnelBuySlot:
                {
                    int64_t cost = GetNextPersonnelSlotCost(&g_GameState);
                    if (g_GameState.cash >= cost)
                    {
                        g_GameState.cash -= cost;
                        g_GameState.purchasedPersonnelSlots++;
                        auto msgBuf = std::make_unique<wchar_t[]>(BUF_LARGE);
                        StringCchPrintfW(msgBuf.get(), BUF_LARGE, L"Purchased additional personnel slot!");
                        AddEventLog(&g_GameState, msgBuf.get(), Colors::Cyan);
                        AddToast(&g_GameState, msgBuf.get(), Colors::Cyan);
                    }
                    break;
                }

                case HitResult::DocStudy:
                {
                    int docIdx = hit.index;
                    int passes = g_GameState.docStudyPasses[docIdx];
                    if (false == g_GameState.docStudyActive && passes < MAX_DOC_PASSES)
                    {
                        const DocStudyDef& doc = GetDocDef(docIdx);
                        int64_t cost = (0 == passes) ? doc.cost : 0;
                        if (g_GameState.cash >= cost)
                        {
                            g_GameState.cash -= cost;
                            g_GameState.docStudyActive = true;
                            g_GameState.currentDocStudy = static_cast<DocId>(docIdx);
                            // Gradual time reduction: pass 0 = 100%, pass 100 = 50%
                            constexpr float kStudyTimeDecayPerPass = 0.005f;
                            constexpr float kStudyTimeMinMult = 0.50f;
                            float timeMult = 1.0f - (passes * kStudyTimeDecayPerPass);
                            if (timeMult < kStudyTimeMinMult)
                            {
                                timeMult = kStudyTimeMinMult;
                            }
                            g_GameState.docStudyRemaining = static_cast<int>(doc.studyTimeSeconds * timeMult);
                            g_GameState.docStudyTotal = g_GameState.docStudyRemaining;
                            auto msgBuf = std::make_unique<wchar_t[]>(BUF_LARGE);
                            StringCchPrintfW(msgBuf.get(), BUF_LARGE, L"Started studying: %s (Pass %d/%d)", doc.name, passes + 1, MAX_DOC_PASSES);
                            AddEventLog(&g_GameState, msgBuf.get(), Colors::Purple);
                        }
                    }
                    break;
                }

                case HitResult::MarketBuy:
                    BuyMarketItem(&g_GameState, hit.index);
                    break;

                case HitResult::MarketSell:
                    SellMarketItem(&g_GameState, hit.index);
                    break;

                case HitResult::PrestigeActivate:
                    ActivatePrestige(&g_GameState);
                    break;

                case HitResult::ResetSave:
                {
                    int confirm1 = MessageBoxW(hWnd, L"Are you sure you want to reset your save?\n\nAll progress will be permanently lost.", L"Reset Save", MB_YESNO | MB_ICONWARNING | MB_DEFBUTTON2);
                    if (IDYES == confirm1)
                    {
                        int confirm2 = MessageBoxW(hWnd, L"This is your FINAL warning.\n\nYour entire save will be deleted. This cannot be undone.\n\nProceed?", L"Confirm Reset", MB_YESNO | MB_ICONERROR | MB_DEFBUTTON2);
                        if (IDYES == confirm2)
                        {
                            DeleteSaveFile();
                            InitGameState(&g_GameState);
                            AddEventLog(&g_GameState, L"Save data wiped. Starting fresh.", Colors::Red);
                            AddToast(&g_GameState, L"Save reset complete.", Colors::Red);
                        }
                    }
                    break;
                }

                case HitResult::BossPrep:
                {
                    int bossIdx = hit.index;
                    int prepIdx = hit.subIndex;
                    const BossHackDef& boss = GetBossHackDef(bossIdx);
                    if (false == g_GameState.bossPrepDone[bossIdx][prepIdx] && g_GameState.cash >= boss.prep[prepIdx].cost)
                    {
                        g_GameState.cash -= boss.prep[prepIdx].cost;
                        g_GameState.bossPrepDone[bossIdx][prepIdx] = true;
                        auto msgBuf = std::make_unique<wchar_t[]>(BUF_LARGE);
                        StringCchPrintfW(msgBuf.get(), BUF_LARGE, L"Preparation complete: %s", boss.prep[prepIdx].name);
                        AddEventLog(&g_GameState, msgBuf.get(), Colors::Orange);
                        AddToast(&g_GameState, msgBuf.get(), Colors::Orange);
                    }
                    break;
                }

                case HitResult::BossAttempt:
                {
                    if (true == CanAttemptBoss(&g_GameState, hit.index))
                    {
                        SpawnBossWindow(&g_GameState, g_hWnd, hit.index);
                    }
                    break;
                }

                case HitResult::IntelBuy:
                {
                    int idx = hit.index;
                    if (idx >= 0 && idx < static_cast<int>(IntelItemType::COUNT))
                    {
                        int64_t price = static_cast<int64_t>(g_IntelItems[idx].basePrice * g_GameState.intelPriceMultipliers[idx]);
                        if (g_GameState.cash >= price && g_GameState.intelMarketStock[idx] > 0 && g_GameState.intelInventory[idx] <= 0)
                        {
                            g_GameState.cash -= price;
                            g_GameState.intelInventory[idx] = 1;
                            g_GameState.intelMarketStock[idx]--;
                            auto msgBuf = std::make_unique<wchar_t[]>(BUF_XLARGE);
                            StringCchPrintfW(msgBuf.get(), BUF_XLARGE, L"Purchased: %s", g_IntelItems[idx].name);
                            AddEventLog(&g_GameState, msgBuf.get(), Colors::Green);
                            AddToast(&g_GameState, msgBuf.get(), Colors::Green);
                        }
                    }
                    break;
                }

                case HitResult::MilitaryEnlist:
                {
                    if (true == g_GameState.militaryEnlistAvailable &&
                        MilitaryPhase::None == g_GameState.militaryPhase &&
                        false == g_GameState.cyberVeteran)
                    {
                        EnlistMilitary(&g_GameState);
                    }
                    break;
                }

                case HitResult::MilitarySpecialtyPick:
                {
                    if (MilitaryPhase::SpecialtyPick == g_GameState.militaryPhase &&
                        hit.index >= 0 && hit.index < NUM_SKILLS)
                    {
                        SelectMilitarySpecialty(&g_GameState, static_cast<SkillId>(hit.index));
                    }
                    break;
                }

                default:
                    break;
            }
            return 0;
        }

        case WM_MOUSEWHEEL:
        {
            int delta = GET_WHEEL_DELTA_WPARAM(wParam);
            int scrollDelta = (delta > 0) ? -kScrollAmount : kScrollAmount;

            switch (g_GameState.currentTab)
            {
                case Tab::Jobs:
                    g_GameState.jobScrollOffset += scrollDelta;
                    if (g_GameState.jobScrollOffset < 0)
                    {
                        g_GameState.jobScrollOffset = 0;
                    }
                    break;
                case Tab::Personnel:
                    g_GameState.personnelScrollOffset += scrollDelta;
                    if (g_GameState.personnelScrollOffset < 0)
                    {
                        g_GameState.personnelScrollOffset = 0;
                    }
                    break;
                case Tab::Skills:
                    g_GameState.skillScrollOffset += scrollDelta;
                    if (g_GameState.skillScrollOffset < 0)
                    {
                        g_GameState.skillScrollOffset = 0;
                    }
                    break;
                case Tab::Market:
                    g_GameState.marketScrollOffset += scrollDelta;
                    if (g_GameState.marketScrollOffset < 0)
                    {
                        g_GameState.marketScrollOffset = 0;
                    }
                    break;
                case Tab::Dashboard:
                    g_GameState.dashboardScrollOffset += scrollDelta;
                    if (g_GameState.dashboardScrollOffset < 0)
                    {
                        g_GameState.dashboardScrollOffset = 0;
                    }
                    break;
                case Tab::Intel:
                    g_GameState.intelScrollOffset += scrollDelta;
                    if (g_GameState.intelScrollOffset < 0)
                    {
                        g_GameState.intelScrollOffset = 0;
                    }
                    break;
                case Tab::Military:
                    g_GameState.militaryScrollOffset += scrollDelta;
                    if (g_GameState.militaryScrollOffset < 0)
                    {
                        g_GameState.militaryScrollOffset = 0;
                    }
                    break;
                default:
                    break;
            }
            return 0;
        }

        case WM_GETMINMAXINFO:
        {
            MINMAXINFO* mmi = reinterpret_cast<MINMAXINFO*>(lParam);
            mmi->ptMinTrackSize.x = kMinWindowWidth;
            mmi->ptMinTrackSize.y = kMinWindowHeight;
            return 0;
        }

        case WM_DESTROY:
            DespawnSpider();
            if (nullptr != g_SpiderWnd)
            {
                DestroyWindow(g_SpiderWnd);
                g_SpiderWnd = NULL;
            }
            HideOverlay();
            if (nullptr != g_OverlayWnd)
            {
                DestroyWindow(g_OverlayWnd);
                g_OverlayWnd = NULL;
            }
            if (nullptr != g_pTaskbar)
            {
                g_pTaskbar->Release();
                g_pTaskbar = nullptr;
            }
            CoUninitialize();

            CleanupIRCWindow();
            CleanupBossWindow();
            CleanupMeltWindow();
            CleanupJailWindow();

            g_GameState.lastSaveTimestamp = static_cast<int64_t>(time(NULL));
            SaveGame(&g_GameState);

            KillTimer(hWnd, TIMER_GAME_TICK);
            KillTimer(hWnd, TIMER_RENDER);
            KillTimer(hWnd, TIMER_TOAST);

            DestroyUIFonts(&g_Fonts);

            if (nullptr != g_MemBitmap)
            {
                DeleteObject(g_MemBitmap);
            }
            if (nullptr != g_MemDC)
            {
                DeleteDC(g_MemDC);
            }

            PostQuitMessage(0);
            return 0;

        case WM_ERASEBKGND:
            return 1;
    }

    return DefWindowProcW(hWnd, msg, wParam, lParam);
}

// ============================================================================
// WinMain
// ============================================================================

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, LPWSTR, int nCmdShow)
{
    InitJobCatalog();

    // Register main window class
    WNDCLASSEXW wc = {};
    wc.cbSize = sizeof(WNDCLASSEXW);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = NULL;
    wc.lpszClassName = L"ShadowNetClass";
    wc.hIcon = LoadIconW(wc.hInstance, MAKEINTRESOURCEW(IDI_GHOST));
    wc.hIconSm = LoadIconW(wc.hInstance, MAKEINTRESOURCEW(IDI_GHOST));

    if (0 == RegisterClassExW(&wc))
    {
        MessageBoxW(NULL, L"Failed to register window class.", L"Error", MB_ICONERROR);
        return 1;
    }

    // Register IRC window class
    WNDCLASSEXW ircWc = {};
    ircWc.cbSize = sizeof(WNDCLASSEXW);
    ircWc.style = CS_HREDRAW | CS_VREDRAW;
    ircWc.lpfnWndProc = IRCWndProc;
    ircWc.hInstance = hInstance;
    ircWc.hCursor = LoadCursor(NULL, IDC_ARROW);
    ircWc.hbrBackground = NULL;
    ircWc.lpszClassName = L"ShadowNetIRCClass";

    if (0 == RegisterClassExW(&ircWc))
    {
        MessageBoxW(NULL, L"Failed to register IRC window class.", L"Error", MB_ICONERROR);
        return 1;
    }

    // Register Screen Melt window class
    WNDCLASSEXW meltWc = {};
    meltWc.cbSize = sizeof(WNDCLASSEXW);
    meltWc.style = CS_HREDRAW | CS_VREDRAW;
    meltWc.lpfnWndProc = MeltWndProc;
    meltWc.hInstance = hInstance;
    meltWc.hCursor = LoadCursor(NULL, IDC_ARROW);
    meltWc.hbrBackground = static_cast<HBRUSH>(GetStockObject(BLACK_BRUSH));
    meltWc.lpszClassName = L"ShadowNetMeltClass";
    RegisterClassExW(&meltWc);

    // Register Jail window class
    WNDCLASSEXW jailWc = {};
    jailWc.cbSize = sizeof(WNDCLASSEXW);
    jailWc.style = CS_HREDRAW | CS_VREDRAW;
    jailWc.lpfnWndProc = JailWndProc;
    jailWc.hInstance = hInstance;
    jailWc.hCursor = LoadCursor(NULL, IDC_ARROW);
    jailWc.hbrBackground = static_cast<HBRUSH>(GetStockObject(BLACK_BRUSH));
    jailWc.lpszClassName = L"ShadowNetJailClass";
    RegisterClassExW(&jailWc);

    // Register Boss window class
    WNDCLASSEXW bossWc = {};
    bossWc.cbSize = sizeof(WNDCLASSEXW);
    bossWc.style = CS_HREDRAW | CS_VREDRAW;
    bossWc.lpfnWndProc = BossWndProc;
    bossWc.hInstance = hInstance;
    bossWc.hCursor = LoadCursor(NULL, IDC_ARROW);
    bossWc.hbrBackground = NULL;
    bossWc.lpszClassName = L"ShadowNetBossClass";

    if (0 == RegisterClassExW(&bossWc))
    {
        MessageBoxW(NULL, L"Failed to register Boss window class.", L"Error", MB_ICONERROR);
        return 1;
    }

    int screenW = GetSystemMetrics(SM_CXSCREEN);
    int screenH = GetSystemMetrics(SM_CYSCREEN);
    int winX = (screenW - kDefaultWindowWidth) / 2;
    int winY = (screenH - kDefaultWindowHeight) / 2;

    g_hWnd = CreateWindowExW(
        0,
        L"ShadowNetClass",
        L"ShadowNet: Rise of the Zero-Day",
        WS_OVERLAPPEDWINDOW,
        winX, winY, kDefaultWindowWidth, kDefaultWindowHeight,
        NULL, NULL, hInstance, NULL);

    if (nullptr == g_hWnd)
    {
        MessageBoxW(NULL, L"Failed to create window.", L"Error", MB_ICONERROR);
        return 1;
    }

    // Dark title bar
    BOOL useDarkMode = TRUE;
    DwmSetWindowAttribute(g_hWnd, 20 /* DWMWA_USE_IMMERSIVE_DARK_MODE */, &useDarkMode, sizeof(useDarkMode));

    // Register and create overlay window (hidden initially)
    WNDCLASSEXW overlayWc = {};
    overlayWc.cbSize = sizeof(WNDCLASSEXW);
    overlayWc.style = CS_HREDRAW | CS_VREDRAW;
    overlayWc.lpfnWndProc = OverlayWndProc;
    overlayWc.hInstance = hInstance;
    overlayWc.hCursor = LoadCursor(NULL, IDC_HAND);
    overlayWc.hbrBackground = NULL;
    overlayWc.lpszClassName = L"ShadowNetOverlay";
    RegisterClassExW(&overlayWc);

    g_OverlayWnd = CreateWindowExW(
        WS_EX_TOPMOST | WS_EX_TOOLWINDOW,
        L"ShadowNetOverlay",
        NULL,
        WS_POPUP,
        0, 0, kOverlayWidth, kOverlayHeight,
        NULL, NULL, hInstance, NULL);

    DwmSetWindowAttribute(g_OverlayWnd, 20, &useDarkMode, sizeof(useDarkMode));

    // Register and create spider window (layered, click-through, hidden initially)
    WNDCLASSEXW spiderWc = {};
    spiderWc.cbSize = sizeof(WNDCLASSEXW);
    spiderWc.lpfnWndProc = SpiderWndProc;
    spiderWc.hInstance = hInstance;
    spiderWc.lpszClassName = L"ShadowNetSpider";
    RegisterClassExW(&spiderWc);

    g_SpiderWnd = CreateWindowExW(
        WS_EX_TOPMOST | WS_EX_TOOLWINDOW | WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_NOACTIVATE,
        L"ShadowNetSpider",
        NULL,
        WS_POPUP,
        0, 0, kSpiderDrawSize, kSpiderDrawSize,
        NULL, NULL, hInstance, NULL);

    // Init ITaskbarList3 for taskbar progress
    CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
    CoCreateInstance(CLSID_TaskbarList, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&g_pTaskbar));
    if (nullptr != g_pTaskbar)
    {
        g_pTaskbar->HrInit();
    }

    ShowWindow(g_hWnd, nCmdShow);
    UpdateWindow(g_hWnd);

    MSG msg;
    while (GetMessageW(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    return static_cast<int>(msg.wParam);
}
