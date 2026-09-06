#pragma once
#include "../core/game.h"

// ============================================================================
// UI Layout Constants
// ============================================================================

constexpr int SIDEBAR_WIDTH = 160;
constexpr int STATUSBAR_HEIGHT = 32;
constexpr int TAB_HEIGHT = 36;
constexpr int STAGE_HEIGHT = 120;      // animated hacker stage strip above the panel
constexpr int PADDING = 8;
constexpr int LINE_HEIGHT = 20;
constexpr int CARD_PADDING = 12;

// ============================================================================
// Font Management
// ============================================================================

struct UIFonts
{
    HFONT titleFont;      // Consolas Bold 18pt
    HFONT bodyFont;       // Consolas 11pt
    HFONT smallFont;      // Consolas 9pt
    HFONT uiFont;         // Segoe UI 9pt
    HFONT uiBoldFont;     // Segoe UI Bold 10pt
};

void CreateUIFonts(UIFonts* fonts);
void DestroyUIFonts(UIFonts* fonts);

// ============================================================================
// Parameter Structs
// ============================================================================

// Bundles HDC + fonts for all drawing calls
struct DrawCtx
{
    HDC hdc;
    const UIFonts* fonts;
};

// Rectangle region for panel drawing
struct PanelRect
{
    int x, y, w, h;
};

// Parameters for text drawing
struct TextParams
{
    HFONT font;
    const wchar_t* text;
    COLORREF color;
};

// Parameters for progress bars
struct BarParams
{
    RECT rc;
    float progress;
    COLORREF fillColor;
    COLORREF bgColor;
};

// Parameters for ASCII-style progress bars
struct AsciiBarParams
{
    HFONT font;
    POINT pos;
    float progress;
    int totalChars;
    COLORREF fillColor;
    COLORREF bgColor;
};

// Window dimensions
struct WindowSize
{
    int width;
    int height;
};

// ============================================================================
// Drawing Helpers
// ============================================================================

void DrawFilledRect(HDC hdc, RECT rc, COLORREF color);
void DrawTextLine(HDC hdc, const TextParams& params, POINT pos);
void DrawTextLineCentered(HDC hdc, const TextParams& params, RECT rc);
void DrawProgressBar(HDC hdc, const BarParams& params);
void DrawAsciiProgressBar(HDC hdc, const AsciiBarParams& params);
void DrawScanlines(HDC hdc, RECT rc);

// Currency formatting
void FormatCurrency(int64_t amount, wchar_t* buf, int bufSize);
void FormatTime(int seconds, wchar_t* buf, int bufSize);

// ============================================================================
// Panel Drawing Functions
// ============================================================================

void DrawSidebar(const DrawCtx& ctx, const GameState* gs, int windowHeight);
void DrawStatusBar(const DrawCtx& ctx, const GameState* gs, WindowSize ws);
void DrawDashboardPanel(const DrawCtx& ctx, const GameState* gs, PanelRect rc);
void DrawJobsPanel(const DrawCtx& ctx, const GameState* gs, PanelRect rc);
void DrawPersonnelPanel(const DrawCtx& ctx, const GameState* gs, PanelRect rc);
void DrawSkillsPanel(const DrawCtx& ctx, const GameState* gs, PanelRect rc);
void DrawMarketPanel(const DrawCtx& ctx, const GameState* gs, PanelRect rc);
void DrawIntelPanel(const DrawCtx& ctx, const GameState* gs, PanelRect rc);
void DrawMilitaryPanel(const DrawCtx& ctx, const GameState* gs, PanelRect rc);
void DrawPrestigePanel(const DrawCtx& ctx, const GameState* gs, PanelRect rc);
void DrawToasts(const DrawCtx& ctx, const GameState* gs, WindowSize ws);

// ============================================================================
// Hit Testing (for mouse clicks)
// ============================================================================

struct HitResult
{
    enum Type
    {
        None = 0,
        SidebarTab,
        JobStart,
        PersonnelHire,
        PersonnelAssign,
        MarketBuy,
        MarketSell,
        PrestigeActivate,
        JobFilter,
        PersonnelBuySlot,
        DocStudy,
        ResetSave,
        BossAttempt,
        BossPrep,
        IntelBuy,
        MilitaryEnlist,
        MilitarySpecialtyPick,
    } type;
    int index;  // Which item was clicked
    int subIndex; // Sub-item (e.g., which personnel to assign)
};

HitResult HitTest(const GameState* gs, POINT mouse, WindowSize ws);
