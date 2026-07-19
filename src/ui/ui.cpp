#include "ui.h"
#include "colors.h"
#include "../data/strings.h"
#include <strsafe.h>
#include <memory>
#include <cstring>
#include <ctime>

// ============================================================================
// Font Size Constants
// ============================================================================

constexpr int kTitleFontSize = -24;
constexpr int kBodyFontSize = -15;
constexpr int kSmallFontSize = -12;
constexpr int kUiFontSize = -12;
constexpr int kUiBoldFontSize = -14;

// ============================================================================
// Font Management
// ============================================================================

void CreateUIFonts(UIFonts* fonts)
{
    fonts->titleFont = CreateFontW(
        kTitleFontSize, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, FIXED_PITCH | FF_MODERN, L"Consolas");

    fonts->bodyFont = CreateFontW(
        kBodyFontSize, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, FIXED_PITCH | FF_MODERN, L"Consolas");

    fonts->smallFont = CreateFontW(
        kSmallFontSize, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, FIXED_PITCH | FF_MODERN, L"Consolas");

    fonts->uiFont = CreateFontW(
        kUiFontSize, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Segoe UI");

    fonts->uiBoldFont = CreateFontW(
        kUiBoldFontSize, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Segoe UI");
}

void DestroyUIFonts(UIFonts* fonts)
{
    DeleteObject(fonts->titleFont);
    DeleteObject(fonts->bodyFont);
    DeleteObject(fonts->smallFont);
    DeleteObject(fonts->uiFont);
    DeleteObject(fonts->uiBoldFont);
}

// ============================================================================
// Drawing Helpers
// ============================================================================

void DrawFilledRect(HDC hdc, RECT rc, COLORREF color)
{
    HBRUSH brush = CreateSolidBrush(color);
    FillRect(hdc, &rc, brush);
    DeleteObject(brush);
}

void DrawTextLine(HDC hdc, const TextParams& params, POINT pos)
{
    HFONT oldFont = static_cast<HFONT>(SelectObject(hdc, params.font));
    SetTextColor(hdc, params.color);
    SetBkMode(hdc, TRANSPARENT);
    TextOutW(hdc, pos.x, pos.y, params.text, lstrlenW(params.text));
    SelectObject(hdc, oldFont);
}

void DrawTextLineCentered(HDC hdc, const TextParams& params, RECT rc)
{
    HFONT oldFont = static_cast<HFONT>(SelectObject(hdc, params.font));
    SetTextColor(hdc, params.color);
    SetBkMode(hdc, TRANSPARENT);
    DrawTextW(hdc, params.text, -1, &rc, DT_CENTER | DT_SINGLELINE | DT_VCENTER);
    SelectObject(hdc, oldFont);
}

void DrawProgressBar(HDC hdc, const BarParams& params)
{
    DrawFilledRect(hdc, params.rc, params.bgColor);
    int w = params.rc.right - params.rc.left;
    int fillW = static_cast<int>(w * params.progress);
    if (fillW > w)
    {
        fillW = w;
    }
    if (fillW > 0)
    {
        DrawFilledRect(hdc, RECT{params.rc.left, params.rc.top, params.rc.left + fillW, params.rc.bottom}, params.fillColor);
    }
}

void DrawAsciiProgressBar(HDC hdc, const AsciiBarParams& params)
{
    int filled = static_cast<int>(params.totalChars * params.progress);
    if (filled > params.totalChars)
    {
        filled = params.totalChars;
    }

    auto bar = std::make_unique<wchar_t[]>(BUF_MEDIUM);
    bar[0] = L'[';
    for (int i = 0; i < params.totalChars; i++)
    {
        bar[i + 1] = (i < filled) ? L'\x2588' : L'\x2591';
    }
    bar[params.totalChars + 1] = L']';
    bar[params.totalChars + 2] = L'\0';

    DrawTextLine(hdc, TextParams{params.font, bar.get(), (params.progress >= 1.0f) ? params.fillColor : params.bgColor}, params.pos);
}

void DrawScanlines(HDC hdc, RECT rc)
{
    constexpr int kScanlineSpacing = 4;

    HPEN pen = CreatePen(PS_SOLID, 1, RGB(0, 0, 0));
    HPEN oldPen = static_cast<HPEN>(SelectObject(hdc, pen));

    for (int ly = rc.top; ly < rc.bottom; ly += kScanlineSpacing)
    {
        MoveToEx(hdc, rc.left, ly, NULL);
        LineTo(hdc, rc.right, ly);
    }

    SelectObject(hdc, oldPen);
    DeleteObject(pen);
}

void FormatCurrency(int64_t amount, wchar_t* buf, int bufSize)
{
    constexpr int kCommaGroupSize = 3;
    constexpr int kSentinelFirstGroup = 100;

    if (amount < 0)
    {
        buf[0] = L'-';
        buf[1] = L'$';
        amount = -amount;
        auto raw = std::make_unique<wchar_t[]>(BUF_SMALL);
        StringCchPrintfW(raw.get(), BUF_SMALL, L"%lld", amount);
        int len = lstrlenW(raw.get());
        int pos = 2;
        int firstGroup = len % kCommaGroupSize;
        if (0 == firstGroup)
        {
            firstGroup = kCommaGroupSize;
        }
        int digitPos = 0;

        for (int i = 0; i < len && pos < bufSize - 1; i++)
        {
            buf[pos++] = raw[i];
            digitPos++;
            if (digitPos == firstGroup && i < len - 1)
            {
                buf[pos++] = L',';
                firstGroup = kSentinelFirstGroup;
                digitPos = 0;
            }
            else if (digitPos == kCommaGroupSize && i < len - 1)
            {
                buf[pos++] = L',';
                digitPos = 0;
            }
        }
        buf[pos] = L'\0';
        return;
    }

    auto raw = std::make_unique<wchar_t[]>(BUF_SMALL);
    StringCchPrintfW(raw.get(), BUF_SMALL, L"%lld", amount);
    int len = lstrlenW(raw.get());

    buf[0] = L'$';
    int pos = 1;
    int firstGroup = len % kCommaGroupSize;
    if (0 == firstGroup)
    {
        firstGroup = kCommaGroupSize;
    }
    int digitPos = 0;

    for (int i = 0; i < len && pos < bufSize - 1; i++)
    {
        buf[pos++] = raw[i];
        digitPos++;
        if (digitPos == firstGroup && i < len - 1 && pos < bufSize - 1)
        {
            buf[pos++] = L',';
            firstGroup = kSentinelFirstGroup;
            digitPos = 0;
        }
        else if (digitPos == kCommaGroupSize && i < len - 1 && pos < bufSize - 1)
        {
            buf[pos++] = L',';
            digitPos = 0;
        }
    }
    buf[pos] = L'\0';
}

void FormatTime(int seconds, wchar_t* buf, int bufSize)
{
    constexpr int kSecondsPerHour = 3600;
    constexpr int kSecondsPerMinute = 60;

    if (seconds >= kSecondsPerHour)
    {
        StringCchPrintfW(buf, bufSize, L"%d:%02d:%02d", seconds / kSecondsPerHour, (seconds % kSecondsPerHour) / kSecondsPerMinute, seconds % kSecondsPerMinute);
    }
    else
    {
        StringCchPrintfW(buf, bufSize, L"%d:%02d", seconds / kSecondsPerMinute, seconds % kSecondsPerMinute);
    }
}

// ============================================================================
// Sidebar
// ============================================================================

void DrawSidebar(const DrawCtx& ctx, const GameState* gs, int windowHeight)
{
    constexpr int kSidebarPadding = 12;
    constexpr int kTitleX = 12;
    constexpr int kTitleY = 12;
    constexpr int kVersionX = 120;
    constexpr int kVersionY = 14;
    constexpr int kSeparatorY = 38;
    constexpr int kTabStartY = 50;
    constexpr int kTabTextOffsetX = 16;
    constexpr int kTabTextOffsetY = 10;
    constexpr int kAccentBarWidth = 3;
    constexpr int kInfoBottomOffset = 120;
    constexpr int kInfoSectionSpacing = 8;
    constexpr int kInfoLineHeight = 18;
    constexpr int kTitleLabelOffsetX = 60;
    constexpr int kSecondsPerHour = 3600;
    constexpr int kSecondsPerMinute = 60;

    DrawFilledRect(ctx.hdc, RECT{0, 0, SIDEBAR_WIDTH, windowHeight}, Colors::BackgroundSecondary);

    // Title
    DrawTextLine(ctx.hdc, TextParams{ctx.fonts->uiBoldFont, L"SHADOWNET", Colors::Green}, POINT{kTitleX, kTitleY});
    DrawTextLine(ctx.hdc, TextParams{ctx.fonts->smallFont, L"v1.0", Colors::Comment}, POINT{kVersionX, kVersionY});

    // Separator
    DrawFilledRect(ctx.hdc, RECT{kSidebarPadding, kSeparatorY, SIDEBAR_WIDTH - kSidebarPadding, kSeparatorY + 1}, Colors::Comment);

    // Tabs
    int tabY = kTabStartY;
    for (int i = 0; i < static_cast<int>(Tab::COUNT); i++)
    {
        // Hide Military tab until enlistment available, active, or veteran
        if (static_cast<Tab>(i) == Tab::Military &&
            false == gs->militaryEnlistAvailable &&
            MilitaryPhase::None == gs->militaryPhase &&
            false == gs->cyberVeteran)
        {
            continue;
        }

        // Hide Prestige tab until requirements met
        if (static_cast<Tab>(i) == Tab::Prestige && false == ArePrestigeRequirementsMet(gs) && false == gs->inPrestigeMode)
        {
            continue;
        }

        bool selected = (gs->currentTab == static_cast<Tab>(i));

        if (true == selected)
        {
            DrawFilledRect(ctx.hdc, RECT{0, tabY, SIDEBAR_WIDTH, tabY + TAB_HEIGHT}, Colors::BackgroundTertiary);
            DrawFilledRect(ctx.hdc, RECT{0, tabY, kAccentBarWidth, tabY + TAB_HEIGHT}, Colors::Green);
        }

        COLORREF textColor = selected ? Colors::Foreground : Colors::Comment;
        DrawTextLine(ctx.hdc, TextParams{ctx.fonts->uiFont, TabNames[i], textColor}, POINT{kTabTextOffsetX, tabY + kTabTextOffsetY});

        tabY += TAB_HEIGHT;
    }

    // Player info at bottom
    int infoY = windowHeight - kInfoBottomOffset;
    DrawFilledRect(ctx.hdc, RECT{kSidebarPadding, infoY, SIDEBAR_WIDTH - kSidebarPadding, infoY + 1}, Colors::Comment);
    infoY += kInfoSectionSpacing;

    DrawTextLine(ctx.hdc, TextParams{ctx.fonts->smallFont, L"TITLE:", Colors::Comment}, POINT{kSidebarPadding, infoY});
    PlayerTitle title = GetPlayerTitle(gs->reputation);
    DrawTextLine(ctx.hdc, TextParams{ctx.fonts->smallFont, PlayerTitleNames[static_cast<int>(title)], Colors::Pink}, POINT{kTitleLabelOffsetX, infoY});
    infoY += kInfoLineHeight;

    auto buf = std::make_unique<wchar_t[]>(BUF_MEDIUM);
    StringCchPrintfW(buf.get(), BUF_MEDIUM, L"Prestige: %d", gs->prestigeLevel);
    DrawTextLine(ctx.hdc, TextParams{ctx.fonts->smallFont, buf.get(), Colors::Purple}, POINT{kSidebarPadding, infoY});
    infoY += kInfoLineHeight;

    int64_t totalTime = gs->totalTicksPlayed;
    int hours = static_cast<int>(totalTime / kSecondsPerHour);
    int minutes = static_cast<int>((totalTime % kSecondsPerHour) / kSecondsPerMinute);
    StringCchPrintfW(buf.get(), BUF_MEDIUM, L"Time: %dh %dm", hours, minutes);
    DrawTextLine(ctx.hdc, TextParams{ctx.fonts->smallFont, buf.get(), Colors::Comment}, POINT{kSidebarPadding, infoY});
}

// ============================================================================
// Status Bar
// ============================================================================

void DrawStatusBar(const DrawCtx& ctx, const GameState* gs, WindowSize ws)
{
    constexpr int kStatusPadding = 16;
    constexpr int kTextOffsetY = 8;
    constexpr int kCashColumnWidth = 140;
    constexpr int kSeparatorWidth = 16;
    constexpr int kRepColumnWidth = 120;
    constexpr int kHeatColumnWidth = 120;
    constexpr int kHeatBarWidth = 100;
    constexpr int kHeatBarHeight = 12;
    constexpr int kHeatBarOffsetY = 10;
    constexpr int kHeatBarColumnWidth = 120;

    int y = ws.height - STATUSBAR_HEIGHT;

    // Jail overlay: replace entire status bar when jailed
    if (true == gs->jailed)
    {
        DrawFilledRect(ctx.hdc, RECT{SIDEBAR_WIDTH, y, ws.width, y + STATUSBAR_HEIGHT}, Colors::Red);
        DrawFilledRect(ctx.hdc, RECT{SIDEBAR_WIDTH, y, ws.width, y + 1}, RGB(180, 0, 0));

        time_t now = time(NULL);
        int remaining = static_cast<int>(gs->jailEndTimestamp - now);
        if (remaining < 0) { remaining = 0; }
        int hours = remaining / 3600;
        int minutes = (remaining % 3600) / 60;
        int seconds = remaining % 60;

        auto jailBuf = std::make_unique<wchar_t[]>(BUF_LARGE);
        StringCchPrintfW(jailBuf.get(), BUF_LARGE, L"OPERATIONS SUSPENDED - FEDERAL SEIZURE    %02d:%02d:%02d", hours, minutes, seconds);
        RECT jailRect = {SIDEBAR_WIDTH, y, ws.width, y + STATUSBAR_HEIGHT};
        DrawTextLineCentered(ctx.hdc, TextParams{ctx.fonts->uiBoldFont, jailBuf.get(), RGB(255, 255, 255)}, jailRect);
        return;
    }

    DrawFilledRect(ctx.hdc, RECT{SIDEBAR_WIDTH, y, ws.width, y + STATUSBAR_HEIGHT}, Colors::BackgroundSecondary);
    DrawFilledRect(ctx.hdc, RECT{SIDEBAR_WIDTH, y, ws.width, y + 1}, Colors::Comment);

    int x = SIDEBAR_WIDTH + kStatusPadding;

    // Cash
    auto cashBuf = std::make_unique<wchar_t[]>(BUF_MEDIUM);
    FormatCurrency(gs->cash, cashBuf.get(), BUF_MEDIUM);
    DrawTextLine(ctx.hdc, TextParams{ctx.fonts->uiFont, cashBuf.get(), Colors::Green}, POINT{x, y + kTextOffsetY});
    x += kCashColumnWidth;

    // Separator
    DrawTextLine(ctx.hdc, TextParams{ctx.fonts->uiFont, L"|", Colors::Comment}, POINT{x, y + kTextOffsetY});
    x += kSeparatorWidth;

    // Reputation
    auto repBuf = std::make_unique<wchar_t[]>(BUF_MEDIUM);
    StringCchPrintfW(repBuf.get(), BUF_MEDIUM, L"REP: %d", gs->reputation);
    DrawTextLine(ctx.hdc, TextParams{ctx.fonts->uiFont, repBuf.get(), Colors::Pink}, POINT{x, y + kTextOffsetY});
    x += kRepColumnWidth;

    DrawTextLine(ctx.hdc, TextParams{ctx.fonts->uiFont, L"|", Colors::Comment}, POINT{x, y + kTextOffsetY});
    x += kSeparatorWidth;

    // Heat
    auto heatBuf = std::make_unique<wchar_t[]>(BUF_MEDIUM);
    StringCchPrintfW(heatBuf.get(), BUF_MEDIUM, L"HEAT: %d/%d", gs->heat, HEAT_MAX);
    COLORREF heatColor = Colors::Green;
    if (gs->heat >= HEAT_CRITICAL)
    {
        heatColor = Colors::Red;
    }
    else if (gs->heat >= HEAT_DANGER)
    {
        heatColor = Colors::Orange;
    }
    else if (gs->heat >= HEAT_WARNING)
    {
        heatColor = Colors::Yellow;
    }
    DrawTextLine(ctx.hdc, TextParams{ctx.fonts->uiFont, heatBuf.get(), heatColor}, POINT{x, y + kTextOffsetY});
    x += kHeatColumnWidth;

    // Heat bar
    DrawProgressBar(ctx.hdc, BarParams{RECT{x, y + kHeatBarOffsetY, x + kHeatBarWidth, y + kHeatBarOffsetY + kHeatBarHeight}, gs->heat / static_cast<float>(HEAT_MAX), heatColor, Colors::BackgroundPrimary});
    x += kHeatBarColumnWidth;

    // Heat-compounding indicator: shows when operating without protection is
    // building a federal case (multiplies heat from each job).
    if (gs->consecutiveJobsNoProtection > 0 && gs->heatCompoundMultiplier > 1.0f)
    {
        auto compBuf = std::make_unique<wchar_t[]>(BUF_SMALL);
        StringCchPrintfW(compBuf.get(), BUF_SMALL, L"x%.1f", gs->heatCompoundMultiplier);
        DrawTextLine(ctx.hdc, TextParams{ctx.fonts->uiFont, compBuf.get(), Colors::Red}, POINT{x, y + kTextOffsetY});
        x += 44;
    }

    DrawTextLine(ctx.hdc, TextParams{ctx.fonts->uiFont, L"|", Colors::Comment}, POINT{x, y + kTextOffsetY});
    x += kSeparatorWidth;

    // Active jobs count + averaged progress bar
    int activeCount = GetActiveJobCount(gs);
    auto jobsBuf = std::make_unique<wchar_t[]>(BUF_MEDIUM);
    StringCchPrintfW(jobsBuf.get(), BUF_MEDIUM, L"Jobs: %d/%d", activeCount, gs->maxJobSlots);
    DrawTextLine(ctx.hdc, TextParams{ctx.fonts->uiFont, jobsBuf.get(), Colors::Cyan}, POINT{x, y + kTextOffsetY});
    x += 80;

    // Single progress bar: average completion across all active jobs
    constexpr int kJobBarWidth = 100;
    constexpr int kJobBarHeight = 12;

    if (activeCount > 0)
    {
        float totalProgress = 0.0f;
        for (int i = 0; i < MAX_ACTIVE_JOBS; i++)
        {
            if (false == gs->activeJobs[i].active)
            {
                continue;
            }
            const ActiveJob& aj = gs->activeJobs[i];
            float p = 1.0f - (aj.remainingSeconds / static_cast<float>(aj.totalSeconds));
            if (p < 0.0f) p = 0.0f;
            if (p > 1.0f) p = 1.0f;
            totalProgress += p;
        }
        float avgProgress = totalProgress / static_cast<float>(activeCount);

        DrawProgressBar(ctx.hdc, BarParams{RECT{x, y + kHeatBarOffsetY, x + kJobBarWidth, y + kHeatBarOffsetY + kJobBarHeight}, avgProgress, Colors::Cyan, Colors::BackgroundPrimary});
        x += kJobBarWidth;
    }

    // Skill learning progress bar
    if (true == gs->docStudyActive)
    {
        x += kSeparatorWidth;
        DrawTextLine(ctx.hdc, TextParams{ctx.fonts->uiFont, L"|", Colors::Comment}, POINT{x, y + kTextOffsetY});
        x += kSeparatorWidth;

        DrawTextLine(ctx.hdc, TextParams{ctx.fonts->uiFont, L"Skill", Colors::Purple}, POINT{x, y + kTextOffsetY});
        x += 44;

        float studyProgress = 1.0f - (gs->docStudyRemaining / static_cast<float>(gs->docStudyTotal));
        if (studyProgress < 0.0f) studyProgress = 0.0f;
        if (studyProgress > 1.0f) studyProgress = 1.0f;

        DrawProgressBar(ctx.hdc, BarParams{RECT{x, y + kHeatBarOffsetY, x + kJobBarWidth, y + kHeatBarOffsetY + kJobBarHeight}, studyProgress, Colors::Purple, Colors::BackgroundPrimary});
    }
}

// ============================================================================
// Dashboard Panel
// ============================================================================

void DrawDashboardPanel(const DrawCtx& ctx, const GameState* gs, PanelRect rc)
{
    constexpr int kTitleSpacing = 36;
    constexpr int kSeparatorSpacing = 8;
    constexpr int kSectionHeaderSpacing = 24;
    constexpr int kTerminalHeight = 200;
    constexpr int kTerminalBgColor = RGB(30, 31, 41);
    constexpr int kLogPadding = 4;
    constexpr int kLogTextIndent = 8;
    constexpr int kTerminalBottomSpacing = 12;
    constexpr int kJobEntryHeight = 44;
    constexpr int kJobLabelIndent = 4;
    constexpr int kProgressBarChars = 20;
    constexpr int kProgressBarOffsetY = 18;
    constexpr int kProgressTextOffset = 250;
    constexpr int kPercentMultiplier = 100;
    constexpr int kQuickStatsSpacing = 8;
    constexpr int kResetButtonWidth = 130;
    constexpr int kResetButtonHeight = 24;
    constexpr int kResetButtonBottomOffset = 40;
    constexpr int kSecondsPerHour = 3600;

    DrawFilledRect(ctx.hdc, RECT{rc.x, rc.y, rc.x + rc.w, rc.y + rc.h}, Colors::BackgroundPrimary);

    int cx = rc.x + PADDING;
    int cy = rc.y + PADDING - gs->dashboardScrollOffset * LINE_HEIGHT;

    // Title
    DrawTextLine(ctx.hdc, TextParams{ctx.fonts->titleFont, L"DASHBOARD", Colors::Foreground}, POINT{cx, cy});
    cy += kTitleSpacing;

    DrawFilledRect(ctx.hdc, RECT{cx, cy, cx + rc.w - PADDING * 2, cy + 1}, Colors::Comment);
    cy += kSeparatorSpacing;

    // Terminal Log
    DrawTextLine(ctx.hdc, TextParams{ctx.fonts->uiBoldFont, L"\x2500 TERMINAL LOG \x2500", Colors::Comment}, POINT{cx, cy});
    cy += kSectionHeaderSpacing;

    // Draw terminal background
    DrawFilledRect(ctx.hdc, RECT{cx, cy, cx + rc.w - PADDING * 2, cy + kTerminalHeight}, kTerminalBgColor);
    // Border
    HPEN borderPen = CreatePen(PS_SOLID, 1, Colors::Comment);
    HPEN oldPen = static_cast<HPEN>(SelectObject(ctx.hdc, borderPen));
    HBRUSH oldBrush = static_cast<HBRUSH>(SelectObject(ctx.hdc, GetStockObject(NULL_BRUSH)));
    Rectangle(ctx.hdc, cx, cy, cx + rc.w - PADDING * 2, cy + kTerminalHeight);
    SelectObject(ctx.hdc, oldBrush);
    SelectObject(ctx.hdc, oldPen);
    DeleteObject(borderPen);

    // Draw event log entries (most recent at bottom)
    int logLines = (kTerminalHeight - kSeparatorSpacing) / LINE_HEIGHT;
    int startIdx = gs->eventLogCount - logLines;
    if (startIdx < 0)
    {
        startIdx = 0;
    }

    int logY = cy + kLogPadding;
    for (int i = startIdx; i < gs->eventLogCount; i++)
    {
        int idx = (gs->eventLogHead - gs->eventLogCount + i + MAX_EVENT_LOG) % MAX_EVENT_LOG;
        auto line = std::make_unique<wchar_t[]>(BUF_LINE);
        StringCchPrintfW(line.get(), BUF_LINE, L"> %s", gs->eventLog[idx].text);
        DrawTextLine(ctx.hdc, TextParams{ctx.fonts->smallFont, line.get(), gs->eventLog[idx].color}, POINT{cx + kLogTextIndent, logY});
        logY += LINE_HEIGHT;
        if (logY >= cy + kTerminalHeight - LINE_HEIGHT)
        {
            break;
        }
    }

    // Blinking cursor
    if (true == gs->cursorVisible)
    {
        DrawTextLine(ctx.hdc, TextParams{ctx.fonts->smallFont, L"> _", Colors::Green}, POINT{cx + kLogTextIndent, logY});
    }

    cy += kTerminalHeight + kTerminalBottomSpacing;

    // Active Operations
    int activeCount = GetActiveJobCount(gs);
    auto opsTitle = std::make_unique<wchar_t[]>(BUF_MEDIUM);
    StringCchPrintfW(opsTitle.get(), BUF_MEDIUM, L"ACTIVE OPERATIONS (%d/%d)", activeCount, gs->maxJobSlots);
    DrawTextLine(ctx.hdc, TextParams{ctx.fonts->uiBoldFont, opsTitle.get(), Colors::Foreground}, POINT{cx, cy});
    cy += kSectionHeaderSpacing;

    for (int i = 0; i < MAX_ACTIVE_JOBS; i++)
    {
        if (false == gs->activeJobs[i].active)
        {
            continue;
        }

        const JobDef& job = GetJobDef(gs->activeJobs[i].jobDefId);
        float progress = 1.0f - static_cast<float>(gs->activeJobs[i].remainingSeconds) / gs->activeJobs[i].totalSeconds;

        // Job name
        auto label = std::make_unique<wchar_t[]>(BUF_LARGE);
        if (gs->activeJobs[i].assignedPersonnelId >= 0)
        {
            const PersonnelMember& pm = gs->personnel[gs->activeJobs[i].assignedPersonnelId];
            StringCchPrintfW(label.get(), BUF_LARGE, L"Auto: %s (%s)", job.name, pm.name);
        }
        else
        {
            StringCchPrintfW(label.get(), BUF_LARGE, L"%s", job.name);
        }
        DrawTextLine(ctx.hdc, TextParams{ctx.fonts->bodyFont, label.get(), Colors::Foreground}, POINT{cx + kJobLabelIndent, cy});

        // Progress bar
        int barX = cx + kJobLabelIndent;
        DrawAsciiProgressBar(ctx.hdc, AsciiBarParams{ctx.fonts->smallFont, POINT{barX, cy + kProgressBarOffsetY}, progress, kProgressBarChars, Colors::Green, Colors::Comment});

        // Percentage and time
        auto timeBuf = std::make_unique<wchar_t[]>(BUF_SMALL);
        FormatTime(gs->activeJobs[i].remainingSeconds, timeBuf.get(), BUF_SMALL);
        auto progBuf = std::make_unique<wchar_t[]>(BUF_MEDIUM);
        StringCchPrintfW(progBuf.get(), BUF_MEDIUM, L"%d%%  %s", static_cast<int>(progress * kPercentMultiplier), timeBuf.get());
        DrawTextLine(ctx.hdc, TextParams{ctx.fonts->smallFont, progBuf.get(), Colors::Orange}, POINT{barX + kProgressTextOffset, cy + kProgressBarOffsetY});

        cy += kJobEntryHeight;
    }

    if (0 == activeCount)
    {
        DrawTextLine(ctx.hdc, TextParams{ctx.fonts->bodyFont, L"No active operations. Start a job!", Colors::Comment}, POINT{cx + kJobLabelIndent, cy});
        cy += kSectionHeaderSpacing;
    }

    // Skill Learning (doc study)
    if (true == gs->docStudyActive)
    {
        cy += kQuickStatsSpacing;
        const DocStudyDef& doc = GetDocDef(static_cast<int>(gs->currentDocStudy));
        float studyProgress = 1.0f - static_cast<float>(gs->docStudyRemaining) / gs->docStudyTotal;

        auto studyLabel = std::make_unique<wchar_t[]>(BUF_LARGE);
        StringCchPrintfW(studyLabel.get(), BUF_LARGE, L"Studying: %s", doc.name);
        DrawTextLine(ctx.hdc, TextParams{ctx.fonts->bodyFont, studyLabel.get(), Colors::Purple}, POINT{cx + kJobLabelIndent, cy});

        int barX = cx + kJobLabelIndent;
        DrawAsciiProgressBar(ctx.hdc, AsciiBarParams{ctx.fonts->smallFont, POINT{barX, cy + kProgressBarOffsetY}, studyProgress, kProgressBarChars, Colors::Purple, Colors::Comment});

        auto studyTimeBuf = std::make_unique<wchar_t[]>(BUF_SMALL);
        FormatTime(gs->docStudyRemaining, studyTimeBuf.get(), BUF_SMALL);
        auto studyProgBuf = std::make_unique<wchar_t[]>(BUF_MEDIUM);
        StringCchPrintfW(studyProgBuf.get(), BUF_MEDIUM, L"%d%%  %s", static_cast<int>(studyProgress * kPercentMultiplier), studyTimeBuf.get());
        DrawTextLine(ctx.hdc, TextParams{ctx.fonts->smallFont, studyProgBuf.get(), Colors::Orange}, POINT{barX + kProgressTextOffset, cy + kProgressBarOffsetY});

        cy += kJobEntryHeight;
    }

    cy += kQuickStatsSpacing;

    // Quick Stats
    DrawTextLine(ctx.hdc, TextParams{ctx.fonts->uiBoldFont, L"QUICK STATS", Colors::Foreground}, POINT{cx, cy});
    cy += kSectionHeaderSpacing;

    auto statBuf = std::make_unique<wchar_t[]>(BUF_LARGE);

    // Income/hr only in prestige (legitimate) mode
    if (true == gs->inPrestigeMode)
    {
        int64_t incomePerHour = 0;
        for (int i = 0; i < MAX_ACTIVE_JOBS; i++)
        {
            if (false == gs->activeJobs[i].active)
            {
                continue;
            }
            const JobDef& job = GetJobDef(gs->activeJobs[i].jobDefId);
            float payBonus = GetSkillPayoutBonus(gs, gs->activeJobs[i].jobDefId);
            int64_t payout = static_cast<int64_t>(job.basePayoutUsd * (1.0f + payBonus));
            int totalTime = gs->activeJobs[i].totalSeconds;
            if (totalTime > 0)
            {
                incomePerHour += payout * kSecondsPerHour / totalTime;
            }
        }

        auto incomeBuf = std::make_unique<wchar_t[]>(BUF_MEDIUM);
        FormatCurrency(incomePerHour, incomeBuf.get(), BUF_MEDIUM);
        StringCchPrintfW(statBuf.get(), BUF_LARGE, L"Revenue/hr: %s", incomeBuf.get());
        DrawTextLine(ctx.hdc, TextParams{ctx.fonts->bodyFont, statBuf.get(), Colors::Green}, POINT{cx + kJobLabelIndent, cy});
        cy += LINE_HEIGHT;
    }

    // Skills active
    int activeSkills = 0;
    for (int i = 0; i < NUM_SKILLS; i++)
    {
        if (gs->skills[i].level > 0)
        {
            activeSkills++;
        }
    }
    StringCchPrintfW(statBuf.get(), BUF_LARGE, L"Skills: %d active  |  Total Jobs: %d", activeSkills, gs->totalJobsCompleted);
    DrawTextLine(ctx.hdc, TextParams{ctx.fonts->bodyFont, statBuf.get(), Colors::Purple}, POINT{cx + kJobLabelIndent, cy});
    cy += LINE_HEIGHT;

    // Personnel count
    int personnelCount = 0;
    for (int i = 0; i < MAX_PERSONNEL; i++)
    {
        if (true == gs->personnel[i].active)
        {
            personnelCount++;
        }
    }
    StringCchPrintfW(statBuf.get(), BUF_LARGE, L"Personnel: %d/%d", personnelCount, gs->maxPersonnelSlots);
    DrawTextLine(ctx.hdc, TextParams{ctx.fonts->bodyFont, statBuf.get(), Colors::Cyan}, POINT{cx + kJobLabelIndent, cy});
    cy += LINE_HEIGHT;

    auto totalBuf = std::make_unique<wchar_t[]>(BUF_MEDIUM);
    FormatCurrency(gs->totalEarned, totalBuf.get(), BUF_MEDIUM);
    StringCchPrintfW(statBuf.get(), BUF_LARGE, L"Total Earned: %s", totalBuf.get());
    DrawTextLine(ctx.hdc, TextParams{ctx.fonts->bodyFont, statBuf.get(), Colors::Green}, POINT{cx + kJobLabelIndent, cy});

    // Reset save button (fixed position at bottom of panel)
    int resetY = rc.y + rc.h - kResetButtonBottomOffset;
    DrawFilledRect(ctx.hdc, RECT{cx + kJobLabelIndent, resetY, cx + kJobLabelIndent + kResetButtonWidth, resetY + kResetButtonHeight}, Colors::Red);
    DrawTextLineCentered(ctx.hdc, TextParams{ctx.fonts->uiFont, L"RESET SAVE", Colors::Foreground}, RECT{cx + kJobLabelIndent, resetY, cx + kJobLabelIndent + kResetButtonWidth, resetY + kResetButtonHeight});
}

// ============================================================================
// Jobs Panel
// ============================================================================

void DrawJobsPanel(const DrawCtx& ctx, const GameState* gs, PanelRect rc)
{
    constexpr int kTitleSpacing = 36;
    constexpr int kSectionHeaderSpacing = 24;
    constexpr int kSeparatorSpacing = 8;
    constexpr int kFilterTabWidth = 50;
    constexpr int kBossCardHeight = 85;
    constexpr int kBossCardSpacing = 90;
    constexpr int kBossAccentWidth = 3;
    constexpr int kBossTextIndent = 12;
    constexpr int kBossTargetOffsetY = 24;
    constexpr int kBossPrepOffsetY = 42;
    constexpr int kBossPrepButtonStartX = 180;
    constexpr int kBossPrepButtonSpacing = 110;
    constexpr int kBossPrepButtonWidth = 100;
    constexpr int kBossPrepButtonTop = 38;
    constexpr int kBossPrepButtonBottom = 56;
    constexpr int kBossHackButtonWidth = 100;
    constexpr int kBossHackButtonHeight = 22;
    constexpr int kBossHackButtonOffsetX = 110;
    constexpr int kBossHackButtonTop = 58;
    constexpr int kBossHackButtonBottom = 80;
    constexpr int kBossCooldownOffsetY = 62;
    constexpr int kBossMinSpace = 20;
    constexpr int kBossMinHeight = 90;
    constexpr int kJobCardHeight = 62;
    constexpr int kJobCardSpacing = 66;
    constexpr int kJobMinSpace = 20;
    constexpr int kJobMinHeight = 65;
    constexpr int kTierBadgeIndent = 8;
    constexpr int kTierBadgeOffsetY = 4;
    constexpr int kJobNameIndent = 40;
    constexpr int kJobDetailY = 24;
    constexpr int kJobTimeIndent = 40;
    constexpr int kJobPayIndent = 120;
    constexpr int kJobHeatIndent = 220;
    constexpr int kJobRepIndent = 310;
    constexpr int kJobCompIndent = 380;
    constexpr int kStartButtonWidth = 70;
    constexpr int kStartButtonHeight = 24;
    constexpr int kStartButtonOffsetX = 80;
    constexpr int kStartButtonTop = 8;
    constexpr int kStartButtonBottom = 32;
    constexpr int kDescriptionY = 42;
    constexpr int kDescMaxLen = 90;
    constexpr int kDescBufSize = 100;
    constexpr int kDescTruncLen = 91;
    constexpr int kLockedTextIndent = 100;

    DrawFilledRect(ctx.hdc, RECT{rc.x, rc.y, rc.x + rc.w, rc.y + rc.h}, Colors::BackgroundPrimary);

    int cx = rc.x + PADDING;
    int cy = rc.y + PADDING;

    DrawTextLine(ctx.hdc, TextParams{ctx.fonts->titleFont, L"JOBS", Colors::Foreground}, POINT{cx, cy});
    cy += kTitleSpacing;

    // Filter tabs -- only show tiers whose boss has been beaten
    const wchar_t* filters[] = { L"[All]", L"[T4]", L"[T3]", L"[T2]", L"[T1]" };
    constexpr int kFilterCount = 5;
    bool filterVisible[] = { true, true, gs->tierBossCompleted[0], gs->tierBossCompleted[1], gs->tierBossCompleted[2] };
    int filterX = cx;
    for (int i = 0; i < kFilterCount; i++)
    {
        if (false == filterVisible[i])
        {
            continue;
        }
        bool selected = (gs->jobFilterTier == i);
        COLORREF col = selected ? Colors::Cyan : Colors::Comment;
        DrawTextLine(ctx.hdc, TextParams{ctx.fonts->uiFont, filters[i], col}, POINT{filterX, cy});
        filterX += kFilterTabWidth;
    }
    cy += kSectionHeaderSpacing;

    DrawFilledRect(ctx.hdc, RECT{cx, cy, cx + rc.w - PADDING * 2, cy + 1}, Colors::Comment);
    cy += kSeparatorSpacing;

    // Boss Hack card (show if a boss is available)
    for (int b = 0; b < NUM_BOSS_HACKS; b++)
    {
        if (true == gs->tierBossCompleted[b])
        {
            continue;
        }

        const BossHackDef& boss = GetBossHackDef(b);

        // Check if player has enough rep for this tier
        int requiredRep = GetBossRequiredRep(b);
        if (gs->reputation < requiredRep)
        {
            continue;
        }
        if (cy + kBossMinHeight > rc.y + rc.h - kBossMinSpace)
        {
            break;
        }

        // Draw boss card with distinct styling
        DrawFilledRect(ctx.hdc, RECT{cx, cy, cx + rc.w - PADDING * 2, cy + kBossCardHeight}, Colors::BackgroundTertiary);
        DrawFilledRect(ctx.hdc, RECT{cx, cy, cx + kBossAccentWidth, cy + kBossCardHeight}, Colors::Red);

        // Operation name
        DrawTextLine(ctx.hdc, TextParams{ctx.fonts->uiBoldFont, boss.operationName, Colors::Red}, POINT{cx + kBossTextIndent, cy + kTierBadgeOffsetY});

        // Target
        auto targetBuf = std::make_unique<wchar_t[]>(BUF_LARGE);
        StringCchPrintfW(targetBuf.get(), BUF_LARGE, L"Target: %s", boss.targetName);
        DrawTextLine(ctx.hdc, TextParams{ctx.fonts->smallFont, targetBuf.get(), Colors::Orange}, POINT{cx + kBossTextIndent, cy + kBossTargetOffsetY});

        // Prep status
        int prepCount = 0;
        for (int p = 0; p < MAX_BOSS_PREP; p++)
        {
            if (true == gs->bossPrepDone[b][p])
            {
                prepCount++;
            }
        }
        auto prepBuf = std::make_unique<wchar_t[]>(BUF_MEDIUM);
        StringCchPrintfW(prepBuf.get(), BUF_MEDIUM, L"Preparations: %d/%d", prepCount, MAX_BOSS_PREP);
        DrawTextLine(ctx.hdc, TextParams{ctx.fonts->smallFont, prepBuf.get(), Colors::Comment}, POINT{cx + kBossTextIndent, cy + kBossPrepOffsetY});

        // Prep buttons
        for (int p = 0; p < MAX_BOSS_PREP; p++)
        {
            int px = cx + kBossPrepButtonStartX + p * kBossPrepButtonSpacing;
            if (true == gs->bossPrepDone[b][p])
            {
                DrawTextLine(ctx.hdc, TextParams{ctx.fonts->smallFont, boss.prep[p].name, Colors::Green}, POINT{px, cy + kBossPrepOffsetY});
            }
            else
            {
                auto costBuf = std::make_unique<wchar_t[]>(BUF_MEDIUM);
                FormatCurrency(boss.prep[p].cost, costBuf.get(), BUF_MEDIUM);
                bool canAffordPrep = gs->cash >= boss.prep[p].cost;
                COLORREF btnColor = canAffordPrep ? Colors::Orange : Colors::BackgroundSecondary;
                DrawFilledRect(ctx.hdc, RECT{px, cy + kBossPrepButtonTop, px + kBossPrepButtonWidth, cy + kBossPrepButtonBottom}, btnColor);
                auto label = std::make_unique<wchar_t[]>(BUF_MEDIUM);
                StringCchPrintfW(label.get(), BUF_MEDIUM, L"%s %s", boss.prep[p].name, costBuf.get());
                DrawTextLineCentered(ctx.hdc, TextParams{ctx.fonts->smallFont, label.get(), canAffordPrep ? Colors::BackgroundPrimary : Colors::Comment}, RECT{px, cy + kBossPrepButtonTop, px + kBossPrepButtonWidth, cy + kBossPrepButtonBottom});
            }
        }

        // Cooldown or HACK button
        if (gs->bossAttemptCooldown[b] > 0)
        {
            auto cdBuf = std::make_unique<wchar_t[]>(BUF_MEDIUM);
            auto timeBuf = std::make_unique<wchar_t[]>(BUF_SMALL);
            FormatTime(gs->bossAttemptCooldown[b], timeBuf.get(), BUF_SMALL);
            StringCchPrintfW(cdBuf.get(), BUF_MEDIUM, L"COOLDOWN: %s", timeBuf.get());
            DrawTextLine(ctx.hdc, TextParams{ctx.fonts->bodyFont, cdBuf.get(), Colors::Comment}, POINT{cx + kBossTextIndent, cy + kBossCooldownOffsetY});
        }
        else
        {
            int btnX = cx + rc.w - PADDING * 2 - kBossHackButtonOffsetX;
            DrawFilledRect(ctx.hdc, RECT{btnX, cy + kBossHackButtonTop, btnX + kBossHackButtonWidth, cy + kBossHackButtonBottom}, Colors::Red);
            DrawTextLineCentered(ctx.hdc, TextParams{ctx.fonts->uiBoldFont, L"HACK", Colors::Foreground}, RECT{btnX, cy + kBossHackButtonTop, btnX + kBossHackButtonWidth, cy + kBossHackButtonBottom});
        }

        cy += kBossCardSpacing;
    }

    // Job list
    int jobCount = GetJobCount();
    int visibleJobs = 0;

    for (int i = 0; i < jobCount; i++)
    {
        const JobDef& job = GetJobDef(i);

        // Filter by tier
        if (gs->jobFilterTier > 0)
        {
            int filterTier = kFilterCount - gs->jobFilterTier;
            if (static_cast<int>(job.tier) != filterTier)
            {
                continue;
            }
        }

        // Hide jobs from tiers whose boss hasn't been beaten yet
        if (Tier::Three == job.tier && false == gs->tierBossCompleted[0])
        {
            continue;
        }
        if (Tier::Two == job.tier && false == gs->tierBossCompleted[1])
        {
            continue;
        }
        if (Tier::One == job.tier && false == gs->tierBossCompleted[2])
        {
            continue;
        }

        // Filter prestige
        if (true == job.isLegitimate && false == gs->inPrestigeMode)
        {
            continue;
        }
        if (false == job.isLegitimate && true == gs->inPrestigeMode)
        {
            continue;
        }

        // Scroll offset
        if (visibleJobs < gs->jobScrollOffset)
        {
            visibleJobs++;
            continue;
        }

        if (cy + kJobMinHeight > rc.y + rc.h - kJobMinSpace)
        {
            break;
        }

        bool unlocked = gs->jobUnlocked[i];
        bool canStart = CanStartJob(gs, i);

        // Card background
        COLORREF cardBg = unlocked ? Colors::BackgroundSecondary : Colors::BackgroundTertiary;
        DrawFilledRect(ctx.hdc, RECT{cx, cy, cx + rc.w - PADDING * 2, cy + kJobCardHeight}, cardBg);

        // Tier badge
        auto tierBuf = std::make_unique<wchar_t[]>(BUF_TINY);
        StringCchPrintfW(tierBuf.get(), BUF_TINY, L"T%d", static_cast<int>(job.tier));
        COLORREF tierColor = Colors::Comment;
        if (Tier::One == job.tier)
        {
            tierColor = Colors::Red;
        }
        else if (Tier::Two == job.tier)
        {
            tierColor = Colors::Orange;
        }
        else if (Tier::Three == job.tier)
        {
            tierColor = Colors::Yellow;
        }
        DrawTextLine(ctx.hdc, TextParams{ctx.fonts->uiBoldFont, tierBuf.get(), tierColor}, POINT{cx + kTierBadgeIndent, cy + kTierBadgeOffsetY});

        // Job name
        DrawTextLine(ctx.hdc, TextParams{ctx.fonts->bodyFont, job.name, unlocked ? Colors::Foreground : Colors::Comment}, POINT{cx + kJobNameIndent, cy + kTierBadgeOffsetY});

        if (true == unlocked)
        {
            // Time
            float speedBonus = GetSkillSpeedBonus(gs, i);
            int adjustedTime = static_cast<int>(job.baseTimeSeconds * (1.0f - speedBonus));
            auto timeBuf = std::make_unique<wchar_t[]>(BUF_SMALL);
            FormatTime(adjustedTime, timeBuf.get(), BUF_SMALL);
            DrawTextLine(ctx.hdc, TextParams{ctx.fonts->smallFont, timeBuf.get(), Colors::Orange}, POINT{cx + kJobTimeIndent, cy + kJobDetailY});

            // Payout
            auto payBuf = std::make_unique<wchar_t[]>(BUF_MEDIUM);
            float payBonus = GetSkillPayoutBonus(gs, i);
            int64_t adjustedPay = static_cast<int64_t>(job.basePayoutUsd * (1.0f + payBonus));
            FormatCurrency(adjustedPay, payBuf.get(), BUF_MEDIUM);
            DrawTextLine(ctx.hdc, TextParams{ctx.fonts->smallFont, payBuf.get(), Colors::Green}, POINT{cx + kJobPayIndent, cy + kJobDetailY});

            // Heat
            auto heatBuf = std::make_unique<wchar_t[]>(BUF_SMALL);
            StringCchPrintfW(heatBuf.get(), BUF_SMALL, L"Heat: %+d", job.heatGenerated);
            COLORREF hColor = job.heatGenerated > 0 ? Colors::Red : Colors::Green;
            if (0 == job.heatGenerated)
            {
                hColor = Colors::Comment;
            }
            DrawTextLine(ctx.hdc, TextParams{ctx.fonts->smallFont, heatBuf.get(), hColor}, POINT{cx + kJobHeatIndent, cy + kJobDetailY});

            // Rep
            auto repBuf = std::make_unique<wchar_t[]>(BUF_SMALL);
            StringCchPrintfW(repBuf.get(), BUF_SMALL, L"+%d rep", job.repReward);
            DrawTextLine(ctx.hdc, TextParams{ctx.fonts->smallFont, repBuf.get(), Colors::Pink}, POINT{cx + kJobRepIndent, cy + kJobDetailY});

            // Completions
            auto compBuf = std::make_unique<wchar_t[]>(BUF_SMALL);
            StringCchPrintfW(compBuf.get(), BUF_SMALL, L"x%d", gs->jobCompletionCounts[i]);
            DrawTextLine(ctx.hdc, TextParams{ctx.fonts->smallFont, compBuf.get(), Colors::Comment}, POINT{cx + kJobCompIndent, cy + kJobDetailY});

            // Start button, or a short reason why it can't be started right now
            int btnX = cx + rc.w - PADDING * 2 - kStartButtonOffsetX;
            if (true == canStart)
            {
                DrawFilledRect(ctx.hdc, RECT{btnX, cy + kStartButtonTop, btnX + kStartButtonWidth, cy + kStartButtonBottom}, Colors::Green);
                DrawTextLineCentered(ctx.hdc, TextParams{ctx.fonts->uiFont, L"START", Colors::BackgroundPrimary}, RECT{btnX, cy + kStartButtonTop, btnX + kStartButtonWidth, cy + kStartButtonBottom});
            }
            else
            {
                const wchar_t* reason = L"Unavailable";
                if (true == IsMilitaryActive(gs))
                {
                    reason = L"On duty";
                }
                else if (true == IsGameHackJob(i) && gs->gameHackBanTimer > 0)
                {
                    reason = L"Banned";
                }
                else if (JOB_ID_FORUM_TIP_FOLLOWUP == i && false == gs->pendingForumTip)
                {
                    reason = L"Lead gone";
                }
                else if (GetActiveJobCount(gs) >= GetMaxJobSlots(gs))
                {
                    reason = L"Slots full";
                }
                DrawTextLine(ctx.hdc, TextParams{ctx.fonts->smallFont, reason, Colors::Orange}, POINT{btnX - 10, cy + kStartButtonTop + 4});
            }

            // Description (truncated)
            auto desc = std::make_unique<wchar_t[]>(kDescBufSize);
            StringCchCopyW(desc.get(), kDescTruncLen, job.description);
            size_t descLen = 0;
            (void)StringCchLengthW(job.description, BUF_XLARGE, &descLen);
            if (descLen > kDescMaxLen)
            {
                StringCchCatW(desc.get(), kDescBufSize, L"...");
            }
            DrawTextLine(ctx.hdc, TextParams{ctx.fonts->smallFont, desc.get(), Colors::Comment}, POINT{cx + kJobNameIndent, cy + kDescriptionY});
        }
        else
        {
            // Show unlock requirements
            DrawTextLine(ctx.hdc, TextParams{ctx.fonts->smallFont, L"LOCKED", Colors::Red}, POINT{cx + kJobNameIndent, cy + kJobDetailY});

            // Show what's needed
            auto reqBuf = std::make_unique<wchar_t[]>(BUF_XLARGE);
            StringCchCopyW(reqBuf.get(), BUF_XLARGE, L"Requires: ");
            bool first = true;

            // Tier requirement
            int requiredRep = 0;
            if (Tier::Three == job.tier)
            {
                requiredRep = REP_TIER3_UNLOCK;
            }
            else if (Tier::Two == job.tier)
            {
                requiredRep = REP_TIER2_UNLOCK;
            }
            else if (Tier::One == job.tier)
            {
                requiredRep = REP_TIER1_UNLOCK;
            }

            if (requiredRep > gs->reputation)
            {
                auto tmp = std::make_unique<wchar_t[]>(BUF_MEDIUM);
                StringCchPrintfW(tmp.get(), BUF_MEDIUM, L"Rep %d", requiredRep);
                StringCchCatW(reqBuf.get(), BUF_XLARGE, tmp.get());
                first = false;
            }

            for (int s = 0; s < job.numSkillReqs; s++)
            {
                if (gs->skills[job.skillReqs[s].skill].level < job.skillReqs[s].minLevel)
                {
                    if (false == first)
                    {
                        StringCchCatW(reqBuf.get(), BUF_XLARGE, L", ");
                    }
                    auto tmp = std::make_unique<wchar_t[]>(BUF_MEDIUM);
                    StringCchPrintfW(tmp.get(), BUF_MEDIUM, L"%s %d", SkillNames[job.skillReqs[s].skill], job.skillReqs[s].minLevel);
                    StringCchCatW(reqBuf.get(), BUF_XLARGE, tmp.get());
                    first = false;
                }
            }

            // Prerequisite job requirement
            if (job.prerequisiteJobId >= 0)
            {
                int prereqCount = (job.prerequisiteJobCount > 0) ? job.prerequisiteJobCount : 1;
                if (gs->jobCompletionCounts[job.prerequisiteJobId] < prereqCount)
                {
                    if (false == first)
                    {
                        StringCchCatW(reqBuf.get(), BUF_XLARGE, L", ");
                    }
                    const JobDef& prereqJob = GetJobDef(job.prerequisiteJobId);
                    auto tmp = std::make_unique<wchar_t[]>(BUF_LARGE);
                    StringCchPrintfW(tmp.get(), BUF_LARGE, L"Complete: %s", prereqJob.name);
                    StringCchCatW(reqBuf.get(), BUF_XLARGE, tmp.get());
                    first = false;
                }
            }

            // Documentation requirement
            if (DocId::COUNT != job.requiredDoc)
            {
                if (false == gs->docsRead[static_cast<int>(job.requiredDoc)])
                {
                    if (false == first)
                    {
                        StringCchCatW(reqBuf.get(), BUF_XLARGE, L", ");
                    }
                    auto tmp = std::make_unique<wchar_t[]>(BUF_LARGE);
                    StringCchPrintfW(tmp.get(), BUF_LARGE, L"Study: %s", DocNames[static_cast<int>(job.requiredDoc)]);
                    StringCchCatW(reqBuf.get(), BUF_XLARGE, tmp.get());
                    first = false;
                }
            }

            // Nothing printable was appended (e.g. the forum-tip lead job, which
            // is gated only by a transient event, not by rep/skill/doc/prereq).
            if (true == first)
            {
                if (JOB_ID_FORUM_TIP_FOLLOWUP == i)
                {
                    StringCchCatW(reqBuf.get(), BUF_XLARGE, L"an active forum tip lead");
                }
                else
                {
                    StringCchCatW(reqBuf.get(), BUF_XLARGE, L"special conditions");
                }
            }

            DrawTextLine(ctx.hdc, TextParams{ctx.fonts->smallFont, reqBuf.get(), Colors::Comment}, POINT{cx + kLockedTextIndent, cy + kJobDetailY});
        }

        cy += kJobCardSpacing;
        visibleJobs++;
    }
}

// ============================================================================
// Personnel Panel
// ============================================================================

// Personnel type definitions accessor (defined in game.cpp, declared here for reference)
// g_PersonnelTypes declared in game.h

void DrawPersonnelPanel(const DrawCtx& ctx, const GameState* gs, PanelRect rc)
{
    constexpr int kTitleSpacing = 36;
    constexpr int kSectionHeaderSpacing = 24;
    constexpr int kSeparatorSpacing = 8;
    constexpr int kSlotInfoSpacing = 24;
    constexpr int kBuyButtonWidth = 60;
    constexpr int kBuyButtonHeight = 20;
    constexpr int kBuyButtonOffsetX = 70;
    constexpr int kAfterSlotButton = 28;
    constexpr int kCardHeight = 44;
    constexpr int kCardSpacing = 48;
    constexpr int kCardMinSpace = 160;
    constexpr int kCardTextIndent = 8;
    constexpr int kCardNameOffsetY = 4;
    constexpr int kCardTypeOffsetX = 160;
    constexpr int kCardLevelOffsetX = 320;
    constexpr int kCardStatusOffsetY = 24;
    constexpr int kCardLoyaltyOffsetX = 380;
    constexpr int kLoyaltyBarLeft = 380;
    constexpr int kLoyaltyBarRight = 460;
    constexpr int kLoyaltyBarTop = 24;
    constexpr int kLoyaltyBarBottom = 32;
    constexpr float kLoyaltyMaxPercent = 100.0f;
    constexpr int kEmptyPersonnelSpacing = 24;
    constexpr int kHireSpacing = 12;
    constexpr int kHireEntryHeight = 26;
    constexpr int kHireMinSpace = 20;
    constexpr int kHireReqOffsetX = 100;
    constexpr int kHireReqTextOffsetY = 4;

    DrawFilledRect(ctx.hdc, RECT{rc.x, rc.y, rc.x + rc.w, rc.y + rc.h}, Colors::BackgroundPrimary);

    int cx = rc.x + PADDING;
    int cy = rc.y + PADDING - gs->personnelScrollOffset * LINE_HEIGHT;

    DrawTextLine(ctx.hdc, TextParams{ctx.fonts->titleFont, L"PERSONNEL", Colors::Foreground}, POINT{cx, cy});
    cy += kTitleSpacing;

    // Slots info
    int personnelCount = 0;
    for (int i = 0; i < MAX_PERSONNEL; i++)
    {
        if (true == gs->personnel[i].active)
        {
            personnelCount++;
        }
    }

    auto slotBuf = std::make_unique<wchar_t[]>(BUF_MEDIUM);
    StringCchPrintfW(slotBuf.get(), BUF_MEDIUM, L"Slots: %d/%d", personnelCount, gs->maxPersonnelSlots);
    DrawTextLine(ctx.hdc, TextParams{ctx.fonts->uiFont, slotBuf.get(), Colors::Cyan}, POINT{cx, cy});
    cy += kSlotInfoSpacing;

    DrawFilledRect(ctx.hdc, RECT{cx, cy, cx + rc.w - PADDING * 2, cy + 1}, Colors::Comment);
    cy += kSeparatorSpacing;

    // Buy Personnel Slot section
    if (GetMaxPersonnelSlots(gs) < MAX_PERSONNEL)
    {
        DrawTextLine(ctx.hdc, TextParams{ctx.fonts->uiBoldFont, L"EXPAND PERSONNEL CAPACITY", Colors::Foreground}, POINT{cx, cy});
        cy += kSectionHeaderSpacing;

        int64_t slotCost = GetNextPersonnelSlotCost(gs);
        auto costBuf = std::make_unique<wchar_t[]>(BUF_MEDIUM);
        FormatCurrency(slotCost, costBuf.get(), BUF_MEDIUM);
        auto slotLabel = std::make_unique<wchar_t[]>(BUF_LARGE);
        StringCchPrintfW(slotLabel.get(), BUF_LARGE, L"Buy Personnel Slot  %s", costBuf.get());
        DrawTextLine(ctx.hdc, TextParams{ctx.fonts->bodyFont, slotLabel.get(), Colors::Foreground}, POINT{cx + kCardTextIndent, cy});

        bool canAffordSlot = gs->cash >= slotCost;
        int btnX = cx + rc.w - PADDING * 2 - kBuyButtonOffsetX;
        if (true == canAffordSlot)
        {
            DrawFilledRect(ctx.hdc, RECT{btnX, cy, btnX + kBuyButtonWidth, cy + kBuyButtonHeight}, Colors::Green);
            DrawTextLineCentered(ctx.hdc, TextParams{ctx.fonts->uiFont, L"BUY", Colors::BackgroundPrimary}, RECT{btnX, cy, btnX + kBuyButtonWidth, cy + kBuyButtonHeight});
        }
        else
        {
            DrawFilledRect(ctx.hdc, RECT{btnX, cy, btnX + kBuyButtonWidth, cy + kBuyButtonHeight}, Colors::BackgroundTertiary);
            DrawTextLineCentered(ctx.hdc, TextParams{ctx.fonts->uiFont, L"BUY", Colors::Comment}, RECT{btnX, cy, btnX + kBuyButtonWidth, cy + kBuyButtonHeight});
        }

        cy += kAfterSlotButton;
        DrawFilledRect(ctx.hdc, RECT{cx, cy, cx + rc.w - PADDING * 2, cy + 1}, Colors::Comment);
        cy += kSeparatorSpacing;
    }

    // Current personnel
    DrawTextLine(ctx.hdc, TextParams{ctx.fonts->uiBoldFont, L"ACTIVE PERSONNEL", Colors::Foreground}, POINT{cx, cy});
    cy += kSectionHeaderSpacing;

    for (int i = 0; i < MAX_PERSONNEL; i++)
    {
        if (false == gs->personnel[i].active)
        {
            continue;
        }
        if (cy + 50 > rc.y + rc.h - kCardMinSpace)
        {
            break;
        }

        const PersonnelMember& pm = gs->personnel[i];

        DrawFilledRect(ctx.hdc, RECT{cx, cy, cx + rc.w - PADDING * 2, cy + kCardHeight}, Colors::BackgroundSecondary);

        // Name
        DrawTextLine(ctx.hdc, TextParams{ctx.fonts->bodyFont, pm.name, pm.arrested ? Colors::Red : Colors::Foreground}, POINT{cx + kCardTextIndent, cy + kCardNameOffsetY});

        // Type
        DrawTextLine(ctx.hdc, TextParams{ctx.fonts->smallFont, g_PersonnelTypes[static_cast<int>(pm.type)].name, Colors::Cyan}, POINT{cx + kCardTypeOffsetX, cy + kCardNameOffsetY});

        // Specialty (jobs training this skill keep full payout when this member works them)
        auto specBuf = std::make_unique<wchar_t[]>(BUF_LARGE);
        StringCchPrintfW(specBuf.get(), BUF_LARGE, L"Specialty: %s", SkillNames[g_PersonnelTypes[static_cast<int>(pm.type)].specialty]);
        DrawTextLine(ctx.hdc, TextParams{ctx.fonts->smallFont, specBuf.get(), Colors::Comment}, POINT{cx + kCardTypeOffsetX, cy + kCardStatusOffsetY});

        // Level
        auto lvlBuf = std::make_unique<wchar_t[]>(BUF_SMALL);
        StringCchPrintfW(lvlBuf.get(), BUF_SMALL, L"Lv.%d", pm.level);
        DrawTextLine(ctx.hdc, TextParams{ctx.fonts->smallFont, lvlBuf.get(), Colors::Purple}, POINT{cx + kCardLevelOffsetX, cy + kCardNameOffsetY});

        // Status
        if (true == pm.arrested)
        {
            auto aBuf = std::make_unique<wchar_t[]>(BUF_MEDIUM);
            FormatTime(pm.arrestTimer, aBuf.get(), BUF_MEDIUM);
            auto msg = std::make_unique<wchar_t[]>(BUF_LARGE);
            StringCchPrintfW(msg.get(), BUF_LARGE, L"ARRESTED (%s)", aBuf.get());
            DrawTextLine(ctx.hdc, TextParams{ctx.fonts->smallFont, msg.get(), Colors::Red}, POINT{cx + kCardTextIndent, cy + kCardStatusOffsetY});
        }
        else if (pm.assignedJobSlot >= 0 && pm.assignedJobSlot < MAX_ACTIVE_JOBS && true == gs->activeJobs[pm.assignedJobSlot].active)
        {
            const JobDef& job = GetJobDef(gs->activeJobs[pm.assignedJobSlot].jobDefId);
            auto msg = std::make_unique<wchar_t[]>(BUF_LARGE);
            StringCchPrintfW(msg.get(), BUF_LARGE, L"Working: %s", job.name);
            DrawTextLine(ctx.hdc, TextParams{ctx.fonts->smallFont, msg.get(), Colors::Green}, POINT{cx + kCardTextIndent, cy + kCardStatusOffsetY});
        }
        else
        {
            DrawTextLine(ctx.hdc, TextParams{ctx.fonts->smallFont, L"Idle", Colors::Comment}, POINT{cx + kCardTextIndent, cy + kCardStatusOffsetY});
        }

        // Loyalty bar
        auto loyBuf = std::make_unique<wchar_t[]>(BUF_SMALL);
        StringCchPrintfW(loyBuf.get(), BUF_SMALL, L"Loyalty: %d%%", pm.loyalty);
        DrawTextLine(ctx.hdc, TextParams{ctx.fonts->smallFont, loyBuf.get(), Colors::Orange}, POINT{cx + kCardLoyaltyOffsetX, cy + kCardNameOffsetY});
        DrawProgressBar(ctx.hdc, BarParams{RECT{cx + kLoyaltyBarLeft, cy + kLoyaltyBarTop, cx + kLoyaltyBarRight, cy + kLoyaltyBarBottom}, pm.loyalty / kLoyaltyMaxPercent, Colors::Orange, Colors::BackgroundPrimary});

        cy += kCardSpacing;
    }

    if (0 == personnelCount)
    {
        DrawTextLine(ctx.hdc, TextParams{ctx.fonts->bodyFont, L"No personnel hired yet.", Colors::Comment}, POINT{cx + kCardTextIndent, cy});
        cy += kEmptyPersonnelSpacing;
    }

    cy += kHireSpacing;

    // Hiring section
    if (personnelCount < gs->maxPersonnelSlots)
    {
        DrawFilledRect(ctx.hdc, RECT{cx, cy, cx + rc.w - PADDING * 2, cy + 1}, Colors::Comment);
        cy += kSeparatorSpacing;
        DrawTextLine(ctx.hdc, TextParams{ctx.fonts->uiBoldFont, L"HIRE PERSONNEL", Colors::Foreground}, POINT{cx, cy});
        cy += kSectionHeaderSpacing;

        for (int i = 0; i < static_cast<int>(PersonnelType::COUNT); i++)
        {
            if (cy + 30 > rc.y + rc.h - kHireMinSpace)
            {
                break;
            }

            const PersonnelTypeDef& pt = g_PersonnelTypes[i];
            bool canAfford = gs->cash >= pt.cost;
            bool repMet = gs->reputation >= pt.requiredRep;

            auto hireBuf = std::make_unique<wchar_t[]>(BUF_LARGE);
            auto costBuf = std::make_unique<wchar_t[]>(BUF_MEDIUM);
            FormatCurrency(pt.cost, costBuf.get(), BUF_MEDIUM);
            StringCchPrintfW(hireBuf.get(), BUF_LARGE, L"%s  %s", pt.name, costBuf.get());

            DrawTextLine(ctx.hdc, TextParams{ctx.fonts->bodyFont, hireBuf.get(), repMet ? Colors::Foreground : Colors::Comment}, POINT{cx + kCardTextIndent, cy});

            if (true == repMet)
            {
                int btnX = cx + rc.w - PADDING * 2 - kBuyButtonOffsetX;
                if (true == canAfford)
                {
                    DrawFilledRect(ctx.hdc, RECT{btnX, cy, btnX + kBuyButtonWidth, cy + kBuyButtonHeight}, Colors::Cyan);
                    DrawTextLineCentered(ctx.hdc, TextParams{ctx.fonts->uiFont, L"HIRE", Colors::BackgroundPrimary}, RECT{btnX, cy, btnX + kBuyButtonWidth, cy + kBuyButtonHeight});
                }
                else
                {
                    DrawFilledRect(ctx.hdc, RECT{btnX, cy, btnX + kBuyButtonWidth, cy + kBuyButtonHeight}, Colors::Red);
                    DrawTextLineCentered(ctx.hdc, TextParams{ctx.fonts->uiFont, L"HIRE?", Colors::Foreground}, RECT{btnX, cy, btnX + kBuyButtonWidth, cy + kBuyButtonHeight});
                }
            }
            else
            {
                auto reqBuf = std::make_unique<wchar_t[]>(BUF_MEDIUM);
                StringCchPrintfW(reqBuf.get(), BUF_MEDIUM, L"Rep %d", pt.requiredRep);
                DrawTextLine(ctx.hdc, TextParams{ctx.fonts->smallFont, reqBuf.get(), Colors::Comment}, POINT{cx + rc.w - PADDING * 2 - kHireReqOffsetX, cy + kHireReqTextOffsetY});
            }

            cy += kHireEntryHeight;
        }
    }
}

// ============================================================================
// Skills Panel
// ============================================================================

void DrawSkillsPanel(const DrawCtx& ctx, const GameState* gs, PanelRect rc)
{
    constexpr int kTitleSpacing = 36;
    constexpr int kSeparatorSpacing = 8;
    constexpr int kColumnHeaderSpacing = 22;
    constexpr int kSkillRowHeight = 26;
    constexpr int kSkillNameIndent = 4;
    constexpr int kLevelColumnX = 240;
    constexpr int kXpColumnX = 300;
    constexpr int kTierColumnX = 420;
    constexpr int kLevelTextOffsetX = 244;
    constexpr int kXpBarLeft = 300;
    constexpr int kXpBarRight = 410;
    constexpr int kXpBarTop = 4;
    constexpr int kXpBarBottom = 16;
    constexpr int kXpTextOffsetX = 302;
    constexpr int kXpTextOffsetY = 2;
    constexpr int kTierTextOffsetX = 424;
    constexpr int kMaxSkillLevel = 100;
    constexpr int kEliteThreshold = 100;
    constexpr int kExpertThreshold = 75;
    constexpr int kSpecialistThreshold = 50;
    constexpr int kCompetentThreshold = 25;
    constexpr int kDocSectionSpacing = 8;
    constexpr int kDocHeaderSpacing = 24;
    constexpr int kDocRowHeight = 26;
    constexpr int kDocNameIndent = 4;
    constexpr int kDocProgressLeft = 300;
    constexpr int kDocProgressRight = 410;
    constexpr int kDocProgressTop = 4;
    constexpr int kDocProgressBottom = 16;
    constexpr int kDocTimeOffsetX = 416;
    constexpr int kDocTimeOffsetY = 2;
    constexpr int kDocStatusOffsetX = 300;
    constexpr int kDocStatusOffsetY = 2;
    constexpr int kDocCostOffsetX = 300;
    constexpr int kDocCostOffsetY = 2;
    constexpr int kStudyButtonWidth = 60;
    constexpr int kStudyButtonHeight = 18;
    constexpr int kStudyButtonOffsetX = 80;
    constexpr int kStudyButtonTop = 2;
    constexpr int kStudyButtonBottom = 20;
    // Doc tier thresholds are defined in game.h (DOC_TIER_*)

    DrawFilledRect(ctx.hdc, RECT{rc.x, rc.y, rc.x + rc.w, rc.y + rc.h}, Colors::BackgroundPrimary);

    // Clip drawing to panel area
    HRGN clipRgn = CreateRectRgn(rc.x, rc.y, rc.x + rc.w, rc.y + rc.h);
    SelectClipRgn(ctx.hdc, clipRgn);

    int cx = rc.x + PADDING;
    int scrollPixels = gs->skillScrollOffset * kSkillRowHeight;
    int cy = rc.y + PADDING - scrollPixels;

    DrawTextLine(ctx.hdc, TextParams{ctx.fonts->titleFont, L"SKILLS", Colors::Foreground}, POINT{cx, cy});
    cy += kTitleSpacing;

    DrawFilledRect(ctx.hdc, RECT{cx, cy, cx + rc.w - PADDING * 2, cy + 1}, Colors::Comment);
    cy += kSeparatorSpacing;

    // Column headers
    DrawTextLine(ctx.hdc, TextParams{ctx.fonts->uiFont, L"Skill", Colors::Comment}, POINT{cx + kSkillNameIndent, cy});
    DrawTextLine(ctx.hdc, TextParams{ctx.fonts->uiFont, L"Level", Colors::Comment}, POINT{cx + kLevelColumnX, cy});
    DrawTextLine(ctx.hdc, TextParams{ctx.fonts->uiFont, L"XP", Colors::Comment}, POINT{cx + kXpColumnX, cy});
    DrawTextLine(ctx.hdc, TextParams{ctx.fonts->uiFont, L"Tier", Colors::Comment}, POINT{cx + kTierColumnX, cy});
    cy += kColumnHeaderSpacing;

    for (int i = 0; i < NUM_SKILLS; i++)
    {
        // Skip drawing if above or below visible area (but don't break - need cy to advance)
        if (cy > rc.y + rc.h)
        {
            cy += kSkillRowHeight;
            continue;
        }

        const SkillState& sk = gs->skills[i];

        // Skill name
        DrawTextLine(ctx.hdc, TextParams{ctx.fonts->bodyFont, SkillNames[i], Colors::Foreground}, POINT{cx + kSkillNameIndent, cy});

        // Level
        auto lvlBuf = std::make_unique<wchar_t[]>(BUF_TINY);
        StringCchPrintfW(lvlBuf.get(), BUF_TINY, L"%d", sk.level);
        DrawTextLine(ctx.hdc, TextParams{ctx.fonts->bodyFont, lvlBuf.get(), Colors::Purple}, POINT{cx + kLevelTextOffsetX, cy});

        // XP progress
        if (sk.level < kMaxSkillLevel)
        {
            int xpNeeded = XpForNextLevel(sk.level);
            float progress = (xpNeeded > 0) ? static_cast<float>(sk.xp) / xpNeeded : 0.0f;
            DrawProgressBar(ctx.hdc, BarParams{RECT{cx + kXpBarLeft, cy + kXpBarTop, cx + kXpBarRight, cy + kXpBarBottom}, progress, Colors::Purple, Colors::BackgroundSecondary});

            auto xpBuf = std::make_unique<wchar_t[]>(BUF_SMALL);
            StringCchPrintfW(xpBuf.get(), BUF_SMALL, L"%d/%d", sk.xp, xpNeeded);
            DrawTextLine(ctx.hdc, TextParams{ctx.fonts->smallFont, xpBuf.get(), Colors::Foreground}, POINT{cx + kXpTextOffsetX, cy + kXpTextOffsetY});
        }
        else
        {
            DrawTextLine(ctx.hdc, TextParams{ctx.fonts->smallFont, L"MAX", Colors::Yellow}, POINT{cx + kXpColumnX, cy + kXpTextOffsetY});
        }

        // Scarcity tier
        const wchar_t* tierName = L"-";
        COLORREF tierColor = Colors::Comment;
        if (sk.level >= kEliteThreshold)
        {
            tierName = L"Elite";
            tierColor = Colors::Yellow;
        }
        else if (sk.level >= kExpertThreshold)
        {
            tierName = L"Expert";
            tierColor = Colors::Orange;
        }
        else if (sk.level >= kSpecialistThreshold)
        {
            tierName = L"Specialist";
            tierColor = Colors::Cyan;
        }
        else if (sk.level >= kCompetentThreshold)
        {
            tierName = L"Competent";
            tierColor = Colors::Green;
        }
        DrawTextLine(ctx.hdc, TextParams{ctx.fonts->smallFont, tierName, tierColor}, POINT{cx + kTierTextOffsetX, cy});

        cy += kSkillRowHeight;
    }

    // Documentation section
    cy += kDocSectionSpacing;
    DrawFilledRect(ctx.hdc, RECT{cx, cy, cx + rc.w - PADDING * 2, cy + 1}, Colors::Comment);
    cy += kDocSectionSpacing;
    DrawTextLine(ctx.hdc, TextParams{ctx.fonts->uiBoldFont, L"DOCUMENTATION", Colors::Foreground}, POINT{cx, cy});
    cy += kDocHeaderSpacing;

    int docCount = GetDocCount();
    for (int i = 0; i < docCount; i++)
    {
        if (cy > rc.y + rc.h)
        {
            cy += kDocRowHeight;
            continue;
        }

        const DocStudyDef& doc = GetDocDef(i);
        int passes = gs->docStudyPasses[i];
        bool isStudying = (true == gs->docStudyActive && static_cast<int>(gs->currentDocStudy) == i);
        bool fullyMastered = (passes >= MAX_DOC_PASSES);

        // Doc name - color based on progress tier
        COLORREF nameColor = Colors::Foreground;
        if (passes >= DOC_TIER_MASTER)
        {
            nameColor = Colors::Green;
        }
        else if (passes >= DOC_TIER_EXPERT)
        {
            nameColor = Colors::Cyan;
        }
        else if (passes >= DOC_TIER_PROFICIENT)
        {
            nameColor = Colors::Purple;
        }
        else if (passes >= DOC_TIER_COMPETENT)
        {
            nameColor = Colors::Yellow;
        }
        else if (passes >= DOC_TIER_FAMILIAR)
        {
            nameColor = Colors::Orange;
        }
        else if (passes >= DOC_TIER_BEGINNER)
        {
            nameColor = Colors::Comment;
        }

        DrawTextLine(ctx.hdc, TextParams{ctx.fonts->bodyFont, doc.name, nameColor}, POINT{cx + kDocNameIndent, cy});

        if (true == isStudying)
        {
            // Progress bar
            float progress = 0.0f;
            if (gs->docStudyTotal > 0)
            {
                progress = 1.0f - static_cast<float>(gs->docStudyRemaining) / gs->docStudyTotal;
            }
            DrawProgressBar(ctx.hdc, BarParams{RECT{cx + kDocProgressLeft, cy + kDocProgressTop, cx + kDocProgressRight, cy + kDocProgressBottom}, progress, Colors::Cyan, Colors::BackgroundSecondary});

            auto timeBuf = std::make_unique<wchar_t[]>(BUF_SMALL);
            FormatTime(gs->docStudyRemaining, timeBuf.get(), BUF_SMALL);
            DrawTextLine(ctx.hdc, TextParams{ctx.fonts->smallFont, timeBuf.get(), Colors::Orange}, POINT{cx + kDocTimeOffsetX, cy + kDocTimeOffsetY});
        }
        else
        {
            // Show tier and pass status
            const wchar_t* tierLabel = L"Novice";
            if (passes >= DOC_TIER_MASTER) { tierLabel = L"Master"; }
            else if (passes >= DOC_TIER_EXPERT) { tierLabel = L"Expert"; }
            else if (passes >= DOC_TIER_PROFICIENT) { tierLabel = L"Proficient"; }
            else if (passes >= DOC_TIER_COMPETENT) { tierLabel = L"Competent"; }
            else if (passes >= DOC_TIER_FAMILIAR) { tierLabel = L"Familiar"; }
            else if (passes >= DOC_TIER_BEGINNER) { tierLabel = L"Beginner"; }

            if (passes > 0)
            {
                int pct = (passes * 100) / MAX_DOC_PASSES;
                auto statusBuf = std::make_unique<wchar_t[]>(BUF_MEDIUM);
                StringCchPrintfW(statusBuf.get(), BUF_MEDIUM, L"%s (%d/%d - %d%%)", tierLabel, passes, MAX_DOC_PASSES, pct);
                DrawTextLine(ctx.hdc, TextParams{ctx.fonts->smallFont, statusBuf.get(), nameColor}, POINT{cx + kDocStatusOffsetX, cy + kDocStatusOffsetY});
            }

            if (true == fullyMastered)
            {
                // Already displayed tier status above; no study button needed
            }
            else
            {
                // Show cost (only for first pass)
                if (0 == passes && doc.cost > 0)
                {
                    auto costBuf = std::make_unique<wchar_t[]>(BUF_MEDIUM);
                    FormatCurrency(doc.cost, costBuf.get(), BUF_MEDIUM);
                    DrawTextLine(ctx.hdc, TextParams{ctx.fonts->smallFont, costBuf.get(), Colors::Yellow}, POINT{cx + kDocCostOffsetX, cy + kDocCostOffsetY});
                }

                // STUDY button (shows pass number)
                int64_t cost = (0 == passes) ? doc.cost : 0;
                bool canStudy = false == gs->docStudyActive && (0 == cost || gs->cash >= cost);
                int btnX = cx + rc.w - PADDING * 2 - kStudyButtonOffsetX;

                auto btnLabel = std::make_unique<wchar_t[]>(BUF_TINY);
                StringCchPrintfW(btnLabel.get(), BUF_TINY, L"PASS %d", passes + 1);

                if (true == canStudy)
                {
                    DrawFilledRect(ctx.hdc, RECT{btnX, cy + kStudyButtonTop, btnX + kStudyButtonWidth, cy + kStudyButtonBottom}, Colors::Cyan);
                    DrawTextLineCentered(ctx.hdc, TextParams{ctx.fonts->uiFont, btnLabel.get(), Colors::BackgroundPrimary}, RECT{btnX, cy + kStudyButtonTop, btnX + kStudyButtonWidth, cy + kStudyButtonBottom});
                }
                else
                {
                    DrawFilledRect(ctx.hdc, RECT{btnX, cy + kStudyButtonTop, btnX + kStudyButtonWidth, cy + kStudyButtonBottom}, Colors::BackgroundTertiary);
                    DrawTextLineCentered(ctx.hdc, TextParams{ctx.fonts->uiFont, btnLabel.get(), Colors::Comment}, RECT{btnX, cy + kStudyButtonTop, btnX + kStudyButtonWidth, cy + kStudyButtonBottom});
                }
            }
        }

        cy += kDocRowHeight;
    }

    // Restore clip region
    SelectClipRgn(ctx.hdc, NULL);
    DeleteObject(clipRgn);
}

// ============================================================================
// Market Panel
// ============================================================================

extern const MarketItem g_MarketItems[] = {
    { MarketItemType::VPNService,       L"VPN Service",           500,    true,  L"5% heat reduction per job; stops case-building" },
    { MarketItemType::ProxyChain,       L"Proxy Chain",           2000,   true,  L"10% heat reduction per job; stops case-building" },
    { MarketItemType::BurnerPhone,      L"Burner Phone",          100,    true,  L"Stops fed case-building (heat compounding)" },
    { MarketItemType::HardwareDevKit,   L"Hardware Dev Kit",      5000,   true,  L"10% faster on hardware/embedded jobs" },
    { MarketItemType::ZeroDayKit,       L"Zero-Day Exploit Kit",  50000,  true,  L"25% faster on exploit (RE) jobs" },
    { MarketItemType::CustomC2,         L"Custom C2 Framework",   10000,  true,  L"15% faster on malware (Win) jobs" },
    { MarketItemType::DedicatedServer,  L"Dedicated Server",      1000,   true,  L"+2 job slots; passive offline income" },
    { MarketItemType::BulletproofVPS,   L"Bulletproof VPS",       5000,   true,  L"15% heat reduction + 1 job slot; offline income" },
    { MarketItemType::TorHiddenService, L"Tor Hidden Service",    2000,   true,  L"Stops case-building; passive offline income" },
    { MarketItemType::SatelliteUplink,  L"Satellite Uplink",      50000,  true,  L"High passive offline income" },
    // Sell side: only shown when you actually hold the goods.
    { MarketItemType::DataDump,         L"Data Dump",             400,    false, L"Sell stolen data to a broker" },
    { MarketItemType::Exploit,          L"Exploit",               2000,   false, L"Sell a working exploit" },
};
extern const int g_NumMarketItems = _countof(g_MarketItems);

// Owned quantity of a sellable market good (0 for non-stackable buy items).
static int MarketSellableCount(const GameState* gs, MarketItemType type)
{
    switch (type)
    {
        case MarketItemType::DataDump: return gs->inventory.dataDumps;
        case MarketItemType::Exploit:  return gs->inventory.exploits;
        default:                       return 0;
    }
}

void DrawMarketPanel(const DrawCtx& ctx, const GameState* gs, PanelRect rc)
{
    constexpr int kTitleSpacing = 36;
    constexpr int kSeparatorSpacing = 8;
    constexpr int kSectionHeaderSpacing = 24;
    constexpr int kItemHeight = 30;
    constexpr int kItemSpacing = 34;
    constexpr int kItemMinSpace = 20;
    constexpr int kItemTextIndent = 8;
    constexpr int kItemTextOffsetY = 2;
    constexpr int kPriceOffsetX = 240;
    constexpr int kPriceOffsetY = 4;
    constexpr int kDescOffsetX = 340;
    constexpr int kDescOffsetY = 4;
    constexpr int kBuyButtonWidth = 50;
    constexpr int kBuyButtonOffsetX = 60;
    constexpr int kBuyButtonTop = 4;
    constexpr int kBuyButtonBottom = 26;
    constexpr int kInventorySpacing = 12;

    DrawFilledRect(ctx.hdc, RECT{rc.x, rc.y, rc.x + rc.w, rc.y + rc.h}, Colors::BackgroundPrimary);

    int cx = rc.x + PADDING;
    int cy = rc.y + PADDING - gs->marketScrollOffset * LINE_HEIGHT;

    DrawTextLine(ctx.hdc, TextParams{ctx.fonts->titleFont, L"MARKET", Colors::Foreground}, POINT{cx, cy});
    cy += kTitleSpacing;

    DrawFilledRect(ctx.hdc, RECT{cx, cy, cx + rc.w - PADDING * 2, cy + 1}, Colors::Comment);
    cy += kSeparatorSpacing;

    // Buy section
    DrawTextLine(ctx.hdc, TextParams{ctx.fonts->uiBoldFont, L"BUY - TOOLS & INFRASTRUCTURE", Colors::Foreground}, POINT{cx, cy});
    cy += kSectionHeaderSpacing;

    for (int i = 0; i < g_NumMarketItems; i++)
    {
        if (cy + kItemSpacing > rc.y + rc.h - kItemMinSpace)
        {
            break;
        }

        const MarketItem& item = g_MarketItems[i];

        int64_t price = static_cast<int64_t>(item.basePrice * gs->market.priceMultipliers[static_cast<int>(item.type)]);
        int btnX = cx + rc.w - PADDING * 2 - kBuyButtonOffsetX;

        // Sell items only appear when the player is actually holding the goods.
        if (false == item.isBuyable)
        {
            int owned = MarketSellableCount(gs, item.type);
            if (owned <= 0)
            {
                continue; // skip entirely (kept in sync with HitTest)
            }

            DrawFilledRect(ctx.hdc, RECT{cx, cy, cx + rc.w - PADDING * 2, cy + kItemHeight}, Colors::BackgroundSecondary);

            auto nameBuf = std::make_unique<wchar_t[]>(BUF_LARGE);
            StringCchPrintfW(nameBuf.get(), BUF_LARGE, L"%s  (owned: %d)", item.name, owned);
            DrawTextLine(ctx.hdc, TextParams{ctx.fonts->bodyFont, nameBuf.get(), Colors::Yellow}, POINT{cx + kItemTextIndent, cy + kItemTextOffsetY});

            auto priceBuf = std::make_unique<wchar_t[]>(BUF_MEDIUM);
            FormatCurrency(price, priceBuf.get(), BUF_MEDIUM);
            DrawTextLine(ctx.hdc, TextParams{ctx.fonts->smallFont, priceBuf.get(), Colors::Green}, POINT{cx + kPriceOffsetX, cy + kPriceOffsetY});
            DrawTextLine(ctx.hdc, TextParams{ctx.fonts->smallFont, item.description, Colors::Comment}, POINT{cx + kDescOffsetX, cy + kDescOffsetY});

            DrawFilledRect(ctx.hdc, RECT{btnX, cy + kBuyButtonTop, btnX + kBuyButtonWidth, cy + kBuyButtonBottom}, Colors::Orange);
            DrawTextLineCentered(ctx.hdc, TextParams{ctx.fonts->uiFont, L"SELL", Colors::BackgroundPrimary}, RECT{btnX, cy + kBuyButtonTop, btnX + kBuyButtonWidth, cy + kBuyButtonBottom});

            cy += kItemSpacing;
            continue;
        }

        DrawFilledRect(ctx.hdc, RECT{cx, cy, cx + rc.w - PADDING * 2, cy + kItemHeight}, Colors::BackgroundSecondary);

        // Name
        DrawTextLine(ctx.hdc, TextParams{ctx.fonts->bodyFont, item.name, Colors::Foreground}, POINT{cx + kItemTextIndent, cy + kItemTextOffsetY});

        // Price
        auto priceBuf = std::make_unique<wchar_t[]>(BUF_MEDIUM);
        FormatCurrency(price, priceBuf.get(), BUF_MEDIUM);
        DrawTextLine(ctx.hdc, TextParams{ctx.fonts->smallFont, priceBuf.get(), Colors::Green}, POINT{cx + kPriceOffsetX, cy + kPriceOffsetY});

        // Description
        DrawTextLine(ctx.hdc, TextParams{ctx.fonts->smallFont, item.description, Colors::Comment}, POINT{cx + kDescOffsetX, cy + kDescOffsetY});

        // Buy button (always shown, greyed out if unaffordable)
        bool canAfford = gs->cash >= price;
        if (true == canAfford)
        {
            DrawFilledRect(ctx.hdc, RECT{btnX, cy + kBuyButtonTop, btnX + kBuyButtonWidth, cy + kBuyButtonBottom}, Colors::Green);
            DrawTextLineCentered(ctx.hdc, TextParams{ctx.fonts->uiFont, L"BUY", Colors::BackgroundPrimary}, RECT{btnX, cy + kBuyButtonTop, btnX + kBuyButtonWidth, cy + kBuyButtonBottom});
        }
        else
        {
            DrawFilledRect(ctx.hdc, RECT{btnX, cy + kBuyButtonTop, btnX + kBuyButtonWidth, cy + kBuyButtonBottom}, Colors::BackgroundTertiary);
            DrawTextLineCentered(ctx.hdc, TextParams{ctx.fonts->uiFont, L"BUY", Colors::Comment}, RECT{btnX, cy + kBuyButtonTop, btnX + kBuyButtonWidth, cy + kBuyButtonBottom});
        }

        cy += kItemSpacing;
    }

    cy += kInventorySpacing;

    // Inventory section
    DrawTextLine(ctx.hdc, TextParams{ctx.fonts->uiBoldFont, L"INVENTORY", Colors::Foreground}, POINT{cx, cy});
    cy += kSectionHeaderSpacing;

    auto invBuf = std::make_unique<wchar_t[]>(BUF_LARGE);

    if (true == gs->inventory.hasVPN)
    {
        DrawTextLine(ctx.hdc, TextParams{ctx.fonts->bodyFont, L"  VPN Service [active]", Colors::Green}, POINT{cx, cy});
        cy += LINE_HEIGHT;
    }
    if (true == gs->inventory.hasProxyChain)
    {
        DrawTextLine(ctx.hdc, TextParams{ctx.fonts->bodyFont, L"  Proxy Chain [active]", Colors::Green}, POINT{cx, cy});
        cy += LINE_HEIGHT;
    }
    if (gs->inventory.burnerPhones > 0)
    {
        StringCchPrintfW(invBuf.get(), BUF_LARGE, L"  Burner Phones: %d", gs->inventory.burnerPhones);
        DrawTextLine(ctx.hdc, TextParams{ctx.fonts->bodyFont, invBuf.get(), Colors::Foreground}, POINT{cx, cy});
        cy += LINE_HEIGHT;
    }
    if (true == gs->inventory.hasHardwareDevKit)
    {
        DrawTextLine(ctx.hdc, TextParams{ctx.fonts->bodyFont, L"  Hardware Dev Kit [active]", Colors::Green}, POINT{cx, cy});
        cy += LINE_HEIGHT;
    }
    if (true == gs->inventory.hasZeroDayKit)
    {
        DrawTextLine(ctx.hdc, TextParams{ctx.fonts->bodyFont, L"  Zero-Day Exploit Kit [active]", Colors::Green}, POINT{cx, cy});
        cy += LINE_HEIGHT;
    }
    if (true == gs->inventory.hasCustomC2)
    {
        DrawTextLine(ctx.hdc, TextParams{ctx.fonts->bodyFont, L"  Custom C2 Framework [active]", Colors::Green}, POINT{cx, cy});
        cy += LINE_HEIGHT;
    }
    if (gs->inventory.dedicatedServers > 0)
    {
        StringCchPrintfW(invBuf.get(), BUF_LARGE, L"  Dedicated Servers: %d", gs->inventory.dedicatedServers);
        DrawTextLine(ctx.hdc, TextParams{ctx.fonts->bodyFont, invBuf.get(), Colors::Foreground}, POINT{cx, cy});
        cy += LINE_HEIGHT;
    }
    if (gs->inventory.bulletproofVPS > 0)
    {
        StringCchPrintfW(invBuf.get(), BUF_LARGE, L"  Bulletproof VPS: %d", gs->inventory.bulletproofVPS);
        DrawTextLine(ctx.hdc, TextParams{ctx.fonts->bodyFont, invBuf.get(), Colors::Foreground}, POINT{cx, cy});
        cy += LINE_HEIGHT;
    }
    if (true == gs->inventory.hasTorHiddenService)
    {
        DrawTextLine(ctx.hdc, TextParams{ctx.fonts->bodyFont, L"  Tor Hidden Service [active]", Colors::Green}, POINT{cx, cy});
        cy += LINE_HEIGHT;
    }
    if (true == gs->inventory.hasSatelliteUplink)
    {
        DrawTextLine(ctx.hdc, TextParams{ctx.fonts->bodyFont, L"  Satellite Uplink [active]", Colors::Green}, POINT{cx, cy});
        cy += LINE_HEIGHT;
    }

    // Data/exploit inventory
    if (gs->inventory.dataDumps > 0)
    {
        StringCchPrintfW(invBuf.get(), BUF_LARGE, L"  Data Dumps: %d", gs->inventory.dataDumps);
        DrawTextLine(ctx.hdc, TextParams{ctx.fonts->bodyFont, invBuf.get(), Colors::Yellow}, POINT{cx, cy});
        cy += LINE_HEIGHT;
    }
    if (gs->inventory.exploits > 0)
    {
        StringCchPrintfW(invBuf.get(), BUF_LARGE, L"  Exploits: %d", gs->inventory.exploits);
        DrawTextLine(ctx.hdc, TextParams{ctx.fonts->bodyFont, invBuf.get(), Colors::Yellow}, POINT{cx, cy});
        cy += LINE_HEIGHT;
    }
}

// ============================================================================
// Intel Panel - Underground Market (Breached Forums style)
// ============================================================================

void DrawIntelPanel(const DrawCtx& ctx, const GameState* gs, PanelRect rc)
{
    constexpr int kTitleSpacing = 36;
    constexpr int kSeparatorSpacing = 8;
    constexpr int kStashHeaderSpacing = 20;
    constexpr int kCategorySpacing = 4;
    constexpr int kCategoryHeaderHeight = 22;
    constexpr int kItemNameIndent = 8;
    constexpr int kPriceOffsetX = 320;
    constexpr int kStockOffsetX = 440;
    constexpr int kBuyButtonX = 540;
    constexpr int kBuyButtonWidth = 50;
    constexpr int kBuyButtonHeight = 18;
    constexpr int kDescIndent = 16;
    constexpr int kFooterSpacing = 16;
    constexpr int kFooterTextIndent = 4;

    DrawFilledRect(ctx.hdc, RECT{rc.x, rc.y, rc.x + rc.w, rc.y + rc.h}, Colors::BackgroundPrimary);

    // Clip to panel bounds so nothing bleeds into the status bar
    HRGN clipRgn = CreateRectRgn(rc.x, rc.y, rc.x + rc.w, rc.y + rc.h);
    SelectClipRgn(ctx.hdc, clipRgn);

    int cx = rc.x + PADDING;
    int cy = rc.y + PADDING;

    DrawTextLine(ctx.hdc, TextParams{ctx.fonts->titleFont, L"UNDERGROUND MARKET", Colors::Red}, POINT{cx, cy});
    cy += kTitleSpacing;

    DrawTextLine(ctx.hdc, TextParams{ctx.fonts->bodyFont, L"Welcome back. Fresh dumps available.", Colors::Comment}, POINT{cx + kFooterTextIndent, cy});
    cy += LINE_HEIGHT;

    DrawFilledRect(ctx.hdc, RECT{cx, cy, cx + rc.w - PADDING * 2, cy + 1}, Colors::Red);
    cy += kSeparatorSpacing;

    // Show intel inventory summary if any owned
    bool hasIntel = false;
    for (int i = 0; i < static_cast<int>(IntelItemType::COUNT); i++)
    {
        if (gs->intelInventory[i] > 0)
        {
            hasIntel = true;
            break;
        }
    }
    if (true == hasIntel)
    {
        DrawTextLine(ctx.hdc, TextParams{ctx.fonts->uiBoldFont, L"YOUR STASH:", Colors::Orange}, POINT{cx, cy});
        cy += kStashHeaderSpacing;
        auto buf = std::make_unique<wchar_t[]>(BUF_XLARGE);
        for (int i = 0; i < static_cast<int>(IntelItemType::COUNT); i++)
        {
            if (gs->intelInventory[i] > 0)
            {
                StringCchPrintfW(buf.get(), BUF_XLARGE, L"  %s  x%d", g_IntelItems[i].name, gs->intelInventory[i]);
                DrawTextLine(ctx.hdc, TextParams{ctx.fonts->bodyFont, buf.get(), Colors::Green}, POINT{cx, cy});
                cy += LINE_HEIGHT;
            }
        }
        cy += kSeparatorSpacing;
        DrawFilledRect(ctx.hdc, RECT{cx, cy, cx + rc.w - PADDING * 2, cy + 1}, Colors::BackgroundSecondary);
        cy += kSeparatorSpacing;
    }

    // Clip scrollable area below the stash so items don't bleed into it
    int scrollAreaTop = cy;
    SelectClipRgn(ctx.hdc, NULL);
    DeleteObject(clipRgn);
    clipRgn = CreateRectRgn(rc.x, scrollAreaTop, rc.x + rc.w, rc.y + rc.h);
    SelectClipRgn(ctx.hdc, clipRgn);

    // Category headers and item listing
    const wchar_t* lastCategory = L"";
    auto priceBuf = std::make_unique<wchar_t[]>(BUF_MEDIUM);
    auto stockBuf = std::make_unique<wchar_t[]>(BUF_MEDIUM);
    auto lineBuf = std::make_unique<wchar_t[]>(BUF_XLARGE);

    // Apply scroll offset
    cy -= gs->intelScrollOffset * LINE_HEIGHT;

    for (int i = 0; i < static_cast<int>(IntelItemType::COUNT); i++)
    {
        const IntelItemDef& item = g_IntelItems[i];

        // Category header
        if (0 != lstrcmpW(item.category, lastCategory))
        {
            lastCategory = item.category;
            cy += kCategorySpacing;
            if (cy + kCategoryHeaderHeight > rc.y && cy < rc.y + rc.h)
            {
                DrawTextLine(ctx.hdc, TextParams{ctx.fonts->uiBoldFont, item.category, Colors::Red}, POINT{cx, cy});
            }
            cy += kCategoryHeaderHeight;
        }

        // Check rep requirement
        bool repOk = gs->reputation >= item.repRequired;
        int64_t price = static_cast<int64_t>(item.basePrice * gs->intelPriceMultipliers[i]);
        bool canAfford = gs->cash >= price;
        int stock = gs->intelMarketStock[i];
        bool alreadyOwned = gs->intelInventory[i] > 0;

        bool visible = (cy + LINE_HEIGHT * 2 > rc.y && cy < rc.y + rc.h);

        if (false == repOk)
        {
            if (true == visible)
            {
                StringCchPrintfW(lineBuf.get(), BUF_XLARGE, L"  [LOCKED] ??? (Rep %d required)", item.repRequired);
                DrawTextLine(ctx.hdc, TextParams{ctx.fonts->bodyFont, lineBuf.get(), Colors::BackgroundSecondary}, POINT{cx, cy});
            }
            cy += LINE_HEIGHT;
        }
        else
        {
            if (true == visible)
            {
                // Item name
                DrawTextLine(ctx.hdc, TextParams{ctx.fonts->bodyFont, item.name, alreadyOwned ? Colors::Green : Colors::Cyan}, POINT{cx + kItemNameIndent, cy});

                if (true == alreadyOwned)
                {
                    // Show OWNED badge
                    int btnX = cx + kBuyButtonX;
                    DrawTextLine(ctx.hdc, TextParams{ctx.fonts->smallFont, L"OWNED", Colors::Green}, POINT{btnX, cy + 2});
                }
                else
                {
                    // Price
                    FormatCurrency(price, priceBuf.get(), BUF_MEDIUM);
                    COLORREF priceColor = canAfford ? Colors::Green : Colors::Red;
                    DrawTextLine(ctx.hdc, TextParams{ctx.fonts->bodyFont, priceBuf.get(), priceColor}, POINT{cx + kPriceOffsetX, cy});

                    // Stock
                    StringCchPrintfW(stockBuf.get(), BUF_MEDIUM, L"Stock: %d", stock);
                    COLORREF stockColor = (stock > 0) ? Colors::Yellow : Colors::Red;
                    DrawTextLine(ctx.hdc, TextParams{ctx.fonts->bodyFont, stockBuf.get(), stockColor}, POINT{cx + kStockOffsetX, cy});

                    // Buy button
                    if (stock > 0 && true == canAfford)
                    {
                        int btnX = cx + kBuyButtonX;
                        DrawFilledRect(ctx.hdc, RECT{btnX, cy, btnX + kBuyButtonWidth, cy + kBuyButtonHeight}, Colors::Green);
                        DrawTextLineCentered(ctx.hdc, TextParams{ctx.fonts->smallFont, L"BUY", Colors::BackgroundPrimary}, RECT{btnX, cy, btnX + kBuyButtonWidth, cy + kBuyButtonHeight});
                    }
                }
            }

            cy += LINE_HEIGHT;

            // Description
            if (true == visible)
            {
                DrawTextLine(ctx.hdc, TextParams{ctx.fonts->smallFont, item.description, Colors::Comment}, POINT{cx + kDescIndent, cy});
            }
            cy += LINE_HEIGHT;
        }
    }

    // Footer
    cy += kFooterSpacing;
    if (cy + LINE_HEIGHT * 2 > rc.y && cy < rc.y + rc.h)
    {
        DrawFilledRect(ctx.hdc, RECT{cx, cy, cx + rc.w - PADDING * 2, cy + 1}, Colors::Red);
        cy += kSeparatorSpacing;
        DrawTextLine(ctx.hdc, TextParams{ctx.fonts->smallFont, L"All transactions are final. No refunds. Data validity not guaranteed.", Colors::Comment}, POINT{cx + kFooterTextIndent, cy});
        cy += LINE_HEIGHT;
        DrawTextLine(ctx.hdc, TextParams{ctx.fonts->smallFont, L"Stock refreshes periodically. Prices fluctuate with market conditions.", Colors::Comment}, POINT{cx + kFooterTextIndent, cy});
    }

    // Restore clip region
    SelectClipRgn(ctx.hdc, NULL);
    DeleteObject(clipRgn);
}

// ============================================================================
// Military Panel
// ============================================================================

void DrawMilitaryPanel(const DrawCtx& ctx, const GameState* gs, PanelRect rc)
{
    constexpr int kTitleSpacing = 36;
    constexpr int kSeparatorSpacing = 8;
    constexpr int kSectionHeaderSpacing = 24;
    constexpr int kTextIndent = 4;
    constexpr int kEnlistButtonWidth = 300;
    constexpr int kEnlistButtonHeight = 36;
    constexpr int kSkillButtonWidth = 200;
    constexpr int kSkillButtonHeight = 28;
    constexpr int kSkillButtonsPerRow = 3;
    constexpr int kSkillButtonSpacingX = 210;
    constexpr int kProgressBarWidth = 300;
    constexpr int kProgressBarHeight = 14;

    DrawFilledRect(ctx.hdc, RECT{rc.x, rc.y, rc.x + rc.w, rc.y + rc.h}, Colors::BackgroundPrimary);

    int cx = rc.x + PADDING;
    int cy = rc.y + PADDING - gs->militaryScrollOffset * LINE_HEIGHT;

    DrawTextLine(ctx.hdc, TextParams{ctx.fonts->titleFont, L"MILITARY CYBER OPERATIONS", Colors::Olive}, POINT{cx, cy});
    cy += kTitleSpacing;

    DrawFilledRect(ctx.hdc, RECT{cx, cy, cx + rc.w - PADDING * 2, cy + 1}, Colors::Comment);
    cy += kSeparatorSpacing;

    if (MilitaryPhase::None == gs->militaryPhase && true == gs->militaryEnlistAvailable && false == gs->cyberVeteran)
    {
        // Enlistment screen
        DrawTextLine(ctx.hdc, TextParams{ctx.fonts->uiBoldFont, L"ENLIST IN MILITARY CYBER COMMAND", Colors::Olive}, POINT{cx, cy});
        cy += kSectionHeaderSpacing;

        DrawTextLine(ctx.hdc, TextParams{ctx.fonts->bodyFont, L"A recruiter has noticed your skills from the DarkBazaar operation.", Colors::Foreground}, POINT{cx + kTextIndent, cy});
        cy += LINE_HEIGHT;
        DrawTextLine(ctx.hdc, TextParams{ctx.fonts->bodyFont, L"The military offers a fast track to elite cyber capabilities.", Colors::Foreground}, POINT{cx + kTextIndent, cy});
        cy += LINE_HEIGHT * 2;

        DrawTextLine(ctx.hdc, TextParams{ctx.fonts->uiBoldFont, L"Service Terms:", Colors::Foreground}, POINT{cx, cy});
        cy += kSectionHeaderSpacing;

        DrawTextLine(ctx.hdc, TextParams{ctx.fonts->bodyFont, L"  Day 1: Basic cyber training - all skills trained to 65%", Colors::Cyan}, POINT{cx, cy});
        cy += LINE_HEIGHT;
        DrawTextLine(ctx.hdc, TextParams{ctx.fonts->bodyFont, L"  Day 1 end: Choose one specialty skill to master (100%)", Colors::Cyan}, POINT{cx, cy});
        cy += LINE_HEIGHT;
        DrawTextLine(ctx.hdc, TextParams{ctx.fonts->bodyFont, L"  Days 2-4: Active duty (90% compliance, 10% actual cyber work)", Colors::Cyan}, POINT{cx, cy});
        cy += LINE_HEIGHT;
        DrawTextLine(ctx.hdc, TextParams{ctx.fonts->bodyFont, L"  Steady military pay throughout service (biweekly)", Colors::Green}, POINT{cx, cy});
        cy += LINE_HEIGHT * 2;

        DrawTextLine(ctx.hdc, TextParams{ctx.fonts->uiBoldFont, L"Upon Completion:", Colors::Foreground}, POINT{cx, cy});
        cy += kSectionHeaderSpacing;
        DrawTextLine(ctx.hdc, TextParams{ctx.fonts->bodyFont, L"  Cyber Veteran perk - all tiers unlocked automatically", Colors::Green}, POINT{cx, cy});
        cy += LINE_HEIGHT;
        DrawTextLine(ctx.hdc, TextParams{ctx.fonts->bodyFont, L"  New Contractor/Mercenary jobs (extremely high pay + risk)", Colors::Green}, POINT{cx, cy});
        cy += LINE_HEIGHT;
        DrawTextLine(ctx.hdc, TextParams{ctx.fonts->bodyFont, L"  One specialty skill at 100%", Colors::Green}, POINT{cx, cy});
        cy += LINE_HEIGHT * 2;

        DrawTextLine(ctx.hdc, TextParams{ctx.fonts->uiBoldFont, L"WARNING:", Colors::Red}, POINT{cx, cy});
        cy += kSectionHeaderSpacing;
        DrawTextLine(ctx.hdc, TextParams{ctx.fonts->bodyFont, L"  Minimum 4 REAL DAYS of service. All civilian jobs suspended.", Colors::Red}, POINT{cx, cy});
        cy += LINE_HEIGHT;
        DrawTextLine(ctx.hdc, TextParams{ctx.fonts->bodyFont, L"  Contractor jobs carry catastrophic failure risk (jail + total loss).", Colors::Red}, POINT{cx, cy});
        cy += LINE_HEIGHT * 2;

        // Enlist button
        DrawFilledRect(ctx.hdc, RECT{cx + kTextIndent, cy, cx + kTextIndent + kEnlistButtonWidth, cy + kEnlistButtonHeight}, Colors::Olive);
        DrawTextLineCentered(ctx.hdc, TextParams{ctx.fonts->uiBoldFont, L"ENLIST - 4 DAY COMMITMENT", Colors::BackgroundPrimary},
            RECT{cx + kTextIndent, cy, cx + kTextIndent + kEnlistButtonWidth, cy + kEnlistButtonHeight});
    }
    else if (MilitaryPhase::Training == gs->militaryPhase)
    {
        // Training phase display
        DrawTextLine(ctx.hdc, TextParams{ctx.fonts->uiBoldFont, L"BASIC CYBER TRAINING - DAY 1", Colors::Olive}, POINT{cx, cy});
        cy += kSectionHeaderSpacing;

        int64_t now = static_cast<int64_t>(time(NULL));
        int64_t elapsed = now - gs->militaryDayStartTimestamp;
        int64_t remaining = MILITARY_DAY_SECONDS - elapsed;
        if (remaining < 0) remaining = 0;
        int hours = static_cast<int>(remaining / 3600);
        int mins = static_cast<int>((remaining % 3600) / 60);
        int secs = static_cast<int>(remaining % 60);

        auto timeStr = std::make_unique<wchar_t[]>(BUF_MEDIUM);
        StringCchPrintfW(timeStr.get(), BUF_MEDIUM, L"Training time remaining: %02d:%02d:%02d", hours, mins, secs);
        DrawTextLine(ctx.hdc, TextParams{ctx.fonts->bodyFont, timeStr.get(), Colors::Foreground}, POINT{cx + kTextIndent, cy});
        cy += LINE_HEIGHT;

        auto payStr = std::make_unique<wchar_t[]>(BUF_MEDIUM);
        StringCchPrintfW(payStr.get(), BUF_MEDIUM, L"Total military pay earned: $%lld", gs->militaryTotalPay);
        DrawTextLine(ctx.hdc, TextParams{ctx.fonts->bodyFont, payStr.get(), Colors::Green}, POINT{cx + kTextIndent, cy});
        cy += LINE_HEIGHT * 2;

        DrawTextLine(ctx.hdc, TextParams{ctx.fonts->uiBoldFont, L"Skill Training Progress (target: 65%):", Colors::Foreground}, POINT{cx, cy});
        cy += kSectionHeaderSpacing;

        // Show progress bars for all skills
        for (int i = 0; i < NUM_SKILLS; i++)
        {
            int level = gs->skills[i].level;
            int percent = (level * 100) / MILITARY_SKILL_CAP_TRAINING;
            if (percent > 100) percent = 100;

            auto skillStr = std::make_unique<wchar_t[]>(BUF_LARGE);
            StringCchPrintfW(skillStr.get(), BUF_LARGE, L"%-24s Lv.%d", SkillNames[i], level);
            DrawTextLine(ctx.hdc, TextParams{ctx.fonts->smallFont, skillStr.get(), level >= MILITARY_SKILL_CAP_TRAINING ? Colors::Green : Colors::Foreground}, POINT{cx + kTextIndent, cy});

            // Progress bar
            int barX = cx + kProgressBarWidth + 40;
            DrawFilledRect(ctx.hdc, RECT{barX, cy + 2, barX + kProgressBarWidth, cy + 2 + kProgressBarHeight}, Colors::BackgroundSecondary);
            int fillWidth = (kProgressBarWidth * percent) / 100;
            COLORREF barColor = (level >= MILITARY_SKILL_CAP_TRAINING) ? Colors::Green : Colors::Olive;
            DrawFilledRect(ctx.hdc, RECT{barX, cy + 2, barX + fillWidth, cy + 2 + kProgressBarHeight}, barColor);

            cy += LINE_HEIGHT;
        }
    }
    else if (MilitaryPhase::SpecialtyPick == gs->militaryPhase)
    {
        // Specialty selection screen
        DrawTextLine(ctx.hdc, TextParams{ctx.fonts->uiBoldFont, L"CHOOSE YOUR SPECIALTY", Colors::Yellow}, POINT{cx, cy});
        cy += kSectionHeaderSpacing;

        DrawTextLine(ctx.hdc, TextParams{ctx.fonts->bodyFont, L"Basic training complete. All skills at 65%.", Colors::Green}, POINT{cx + kTextIndent, cy});
        cy += LINE_HEIGHT;
        DrawTextLine(ctx.hdc, TextParams{ctx.fonts->bodyFont, L"Select one skill to specialize in. It will be trained to 100% during active duty.", Colors::Foreground}, POINT{cx + kTextIndent, cy});
        cy += LINE_HEIGHT * 2;

        // Skill selection buttons in a grid
        for (int i = 0; i < NUM_SKILLS; i++)
        {
            int col = i % kSkillButtonsPerRow;
            int row = i / kSkillButtonsPerRow;

            int btnX = cx + kTextIndent + col * kSkillButtonSpacingX;
            int btnY = cy + row * (kSkillButtonHeight + 6);

            DrawFilledRect(ctx.hdc, RECT{btnX, btnY, btnX + kSkillButtonWidth, btnY + kSkillButtonHeight}, Colors::BackgroundSecondary);
            DrawTextLineCentered(ctx.hdc, TextParams{ctx.fonts->smallFont, SkillNames[i], Colors::Foreground},
                RECT{btnX, btnY, btnX + kSkillButtonWidth, btnY + kSkillButtonHeight});
        }
    }
    else if (MilitaryPhase::ActiveDuty == gs->militaryPhase)
    {
        // Active duty display
        auto titleStr = std::make_unique<wchar_t[]>(BUF_MEDIUM);
        int dayNum = gs->militaryDaysCompleted + 1;
        StringCchPrintfW(titleStr.get(), BUF_MEDIUM, L"ACTIVE DUTY - DAY %d of %d", dayNum, MILITARY_SERVICE_DAYS);
        DrawTextLine(ctx.hdc, TextParams{ctx.fonts->uiBoldFont, titleStr.get(), Colors::Olive}, POINT{cx, cy});
        cy += kSectionHeaderSpacing;

        int64_t now = static_cast<int64_t>(time(NULL));
        int64_t elapsed = now - gs->militaryDayStartTimestamp;
        int64_t remaining = MILITARY_DAY_SECONDS - elapsed;
        if (remaining < 0) remaining = 0;
        int hours = static_cast<int>(remaining / 3600);
        int mins = static_cast<int>((remaining % 3600) / 60);
        int secs = static_cast<int>(remaining % 60);

        auto timeStr = std::make_unique<wchar_t[]>(BUF_MEDIUM);
        StringCchPrintfW(timeStr.get(), BUF_MEDIUM, L"Current day remaining: %02d:%02d:%02d", hours, mins, secs);
        DrawTextLine(ctx.hdc, TextParams{ctx.fonts->bodyFont, timeStr.get(), Colors::Foreground}, POINT{cx + kTextIndent, cy});
        cy += LINE_HEIGHT;

        int daysLeft = MILITARY_SERVICE_DAYS - gs->militaryDaysCompleted;
        auto daysStr = std::make_unique<wchar_t[]>(BUF_MEDIUM);
        StringCchPrintfW(daysStr.get(), BUF_MEDIUM, L"Days of service remaining: %d", daysLeft);
        DrawTextLine(ctx.hdc, TextParams{ctx.fonts->bodyFont, daysStr.get(), Colors::Foreground}, POINT{cx + kTextIndent, cy});
        cy += LINE_HEIGHT;

        auto payStr = std::make_unique<wchar_t[]>(BUF_MEDIUM);
        StringCchPrintfW(payStr.get(), BUF_MEDIUM, L"Total military pay earned: $%lld", gs->militaryTotalPay);
        DrawTextLine(ctx.hdc, TextParams{ctx.fonts->bodyFont, payStr.get(), Colors::Green}, POINT{cx + kTextIndent, cy});
        cy += LINE_HEIGHT * 2;

        // Specialty progress
        SkillId spec = gs->militarySpecialty;
        int specLevel = gs->skills[spec].level;
        int specPercent = (specLevel * 100) / MILITARY_SKILL_CAP_SPECIALTY;
        if (specPercent > 100) specPercent = 100;

        auto specStr = std::make_unique<wchar_t[]>(BUF_LARGE);
        StringCchPrintfW(specStr.get(), BUF_LARGE, L"Specialty: %s (Lv.%d / %d)", SkillNames[spec], specLevel, MILITARY_SKILL_CAP_SPECIALTY);
        DrawTextLine(ctx.hdc, TextParams{ctx.fonts->uiBoldFont, specStr.get(), Colors::Purple}, POINT{cx, cy});
        cy += kSectionHeaderSpacing;

        // Specialty progress bar
        DrawFilledRect(ctx.hdc, RECT{cx + kTextIndent, cy, cx + kTextIndent + kProgressBarWidth, cy + kProgressBarHeight}, Colors::BackgroundSecondary);
        int fillWidth = (kProgressBarWidth * specPercent) / 100;
        COLORREF barColor = (specLevel >= MILITARY_SKILL_CAP_SPECIALTY) ? Colors::Green : Colors::Purple;
        DrawFilledRect(ctx.hdc, RECT{cx + kTextIndent, cy, cx + kTextIndent + fillWidth, cy + kProgressBarHeight}, barColor);
        cy += LINE_HEIGHT * 2;

        // Duty breakdown
        DrawTextLine(ctx.hdc, TextParams{ctx.fonts->uiBoldFont, L"Daily Breakdown:", Colors::Foreground}, POINT{cx, cy});
        cy += kSectionHeaderSpacing;
        DrawTextLine(ctx.hdc, TextParams{ctx.fonts->bodyFont, L"  90% - Compliance, PT, busywork, unrelated tasks", Colors::Orange}, POINT{cx, cy});
        cy += LINE_HEIGHT;
        DrawTextLine(ctx.hdc, TextParams{ctx.fonts->bodyFont, L"  10% - Actual cyber operations work", Colors::Cyan}, POINT{cx, cy});
        cy += LINE_HEIGHT;
        DrawTextLine(ctx.hdc, TextParams{ctx.fonts->bodyFont, L"  Check event log for duty assignments.", Colors::Comment}, POINT{cx, cy});
    }
    else if (true == gs->cyberVeteran)
    {
        // Veteran status display
        DrawTextLine(ctx.hdc, TextParams{ctx.fonts->uiBoldFont, L"CYBER VETERAN - HONORABLY DISCHARGED", Colors::Green}, POINT{cx, cy});
        cy += kSectionHeaderSpacing;

        DrawTextLine(ctx.hdc, TextParams{ctx.fonts->bodyFont, L"Service complete. DD-214 on file.", Colors::Comment}, POINT{cx + kTextIndent, cy});
        cy += LINE_HEIGHT;

        auto payStr = std::make_unique<wchar_t[]>(BUF_MEDIUM);
        StringCchPrintfW(payStr.get(), BUF_MEDIUM, L"Total military pay earned: $%lld", gs->militaryTotalPay);
        DrawTextLine(ctx.hdc, TextParams{ctx.fonts->bodyFont, payStr.get(), Colors::Green}, POINT{cx + kTextIndent, cy});
        cy += LINE_HEIGHT;

        auto specStr = std::make_unique<wchar_t[]>(BUF_LARGE);
        StringCchPrintfW(specStr.get(), BUF_LARGE, L"Military Specialty: %s (Lv.%d)", SkillNames[gs->militarySpecialty], gs->skills[gs->militarySpecialty].level);
        DrawTextLine(ctx.hdc, TextParams{ctx.fonts->bodyFont, specStr.get(), Colors::Purple}, POINT{cx + kTextIndent, cy});
        cy += LINE_HEIGHT * 2;

        DrawTextLine(ctx.hdc, TextParams{ctx.fonts->uiBoldFont, L"Veteran Perks:", Colors::Foreground}, POINT{cx, cy});
        cy += kSectionHeaderSpacing;
        DrawTextLine(ctx.hdc, TextParams{ctx.fonts->bodyFont, L"  All tiers unlocked (T4 through Shadow Broker)", Colors::Green}, POINT{cx, cy});
        cy += LINE_HEIGHT;
        DrawTextLine(ctx.hdc, TextParams{ctx.fonts->bodyFont, L"  Contractor/Mercenary jobs available in Jobs tab", Colors::Green}, POINT{cx, cy});
        cy += LINE_HEIGHT;
        DrawTextLine(ctx.hdc, TextParams{ctx.fonts->bodyFont, L"  All skills at 65%+ with one specialty at 100%", Colors::Green}, POINT{cx, cy});
        cy += LINE_HEIGHT * 2;

        DrawTextLine(ctx.hdc, TextParams{ctx.fonts->uiBoldFont, L"Contractor Jobs:", Colors::Red}, POINT{cx, cy});
        cy += kSectionHeaderSpacing;
        DrawTextLine(ctx.hdc, TextParams{ctx.fonts->bodyFont, L"  Extremely high pay and massive reputation gains.", Colors::Foreground}, POINT{cx, cy});
        cy += LINE_HEIGHT;
        DrawTextLine(ctx.hdc, TextParams{ctx.fonts->bodyFont, L"  WARNING: Failure = jail + lose ALL money + ALL reputation.", Colors::Red}, POINT{cx, cy});
        cy += LINE_HEIGHT;
        DrawTextLine(ctx.hdc, TextParams{ctx.fonts->bodyFont, L"  Skills and experience are preserved on failure.", Colors::Comment}, POINT{cx, cy});
    }
}

// ============================================================================
// Prestige Panel
// ============================================================================

void DrawPrestigePanel(const DrawCtx& ctx, const GameState* gs, PanelRect rc)
{
    constexpr int kTitleSpacing = 36;
    constexpr int kSeparatorSpacing = 8;
    constexpr int kSectionHeaderSpacing = 24;
    constexpr int kTextIndent = 4;
    constexpr int kPrestigeButtonWidth = 200;
    constexpr int kPrestigeButtonHeight = 32;
    constexpr int kPrestigeButtonSpacing = 48;
    constexpr int kBonusTextOffsetY = 16;

    DrawFilledRect(ctx.hdc, RECT{rc.x, rc.y, rc.x + rc.w, rc.y + rc.h}, Colors::BackgroundPrimary);

    int cx = rc.x + PADDING;
    int cy = rc.y + PADDING;

    DrawTextLine(ctx.hdc, TextParams{ctx.fonts->titleFont, L"PRESTIGE", Colors::Foreground}, POINT{cx, cy});
    cy += kTitleSpacing;

    DrawFilledRect(ctx.hdc, RECT{cx, cy, cx + rc.w - PADDING * 2, cy + 1}, Colors::Comment);
    cy += kSeparatorSpacing;

    if (false == gs->inPrestigeMode)
    {
        // Show prestige requirements
        DrawTextLine(ctx.hdc, TextParams{ctx.fonts->uiBoldFont, L"GO LEGITIMATE", Colors::Pink}, POINT{cx, cy});
        cy += kSectionHeaderSpacing;

        DrawTextLine(ctx.hdc, TextParams{ctx.fonts->bodyFont, L"Cash out and found a cybersecurity company.", Colors::Comment}, POINT{cx + kTextIndent, cy});
        cy += LINE_HEIGHT;
        DrawTextLine(ctx.hdc, TextParams{ctx.fonts->bodyFont, L"The same skills that make you dangerous make you invaluable.", Colors::Comment}, POINT{cx + kTextIndent, cy});
        cy += LINE_HEIGHT * 2;

        DrawTextLine(ctx.hdc, TextParams{ctx.fonts->uiBoldFont, L"Requirements:", Colors::Foreground}, POINT{cx, cy});
        cy += kSectionHeaderSpacing;

        // Rep check
        bool repMet = gs->reputation >= REP_SHADOW_BROKER;
        auto repBuf = std::make_unique<wchar_t[]>(BUF_LARGE);
        StringCchPrintfW(repBuf.get(), BUF_LARGE, L"  Reputation >= 10,000 (current: %d)", gs->reputation);
        DrawTextLine(ctx.hdc, TextParams{ctx.fonts->bodyFont, repBuf.get(), repMet ? Colors::Green : Colors::Red}, POINT{cx, cy});
        cy += LINE_HEIGHT;

        // Tier 1 jobs check
        int tier1Completed = 0;
        int jobCount = GetJobCount();
        for (int i = 0; i < jobCount; i++)
        {
            const JobDef& job = GetJobDef(i);
            if (Tier::One == job.tier && false == job.isLegitimate && gs->jobCompletionCounts[i] > 0)
            {
                tier1Completed++;
            }
        }
        bool jobsMet = tier1Completed >= PRESTIGE_TIER1_JOBS_REQUIRED;
        auto jobsBuf = std::make_unique<wchar_t[]>(BUF_LARGE);
        StringCchPrintfW(jobsBuf.get(), BUF_LARGE, L"  5+ Tier 1 jobs completed (current: %d)", tier1Completed);
        DrawTextLine(ctx.hdc, TextParams{ctx.fonts->bodyFont, jobsBuf.get(), jobsMet ? Colors::Green : Colors::Red}, POINT{cx, cy});
        cy += LINE_HEIGHT;

        // Cash check
        bool cashMet = gs->cash >= PRESTIGE_CASH_REQUIRED;
        auto cashFmt = std::make_unique<wchar_t[]>(BUF_MEDIUM);
        FormatCurrency(gs->cash, cashFmt.get(), BUF_MEDIUM);
        auto cashBuf = std::make_unique<wchar_t[]>(BUF_LARGE);
        StringCchPrintfW(cashBuf.get(), BUF_LARGE, L"  $5,000,000+ cash on hand (current: %s)", cashFmt.get());
        DrawTextLine(ctx.hdc, TextParams{ctx.fonts->bodyFont, cashBuf.get(), cashMet ? Colors::Green : Colors::Red}, POINT{cx, cy});
        cy += LINE_HEIGHT * 2;

        bool canPrestige = repMet && jobsMet && cashMet;
        if (true == canPrestige)
        {
            DrawFilledRect(ctx.hdc, RECT{cx + kTextIndent, cy, cx + kTextIndent + kPrestigeButtonWidth, cy + kPrestigeButtonHeight}, Colors::Pink);
            DrawTextLineCentered(ctx.hdc, TextParams{ctx.fonts->uiBoldFont, L"GO LEGITIMATE", Colors::BackgroundPrimary}, RECT{cx + kTextIndent, cy, cx + kTextIndent + kPrestigeButtonWidth, cy + kPrestigeButtonHeight});
        }

        cy += kPrestigeButtonSpacing;

        // Prestige bonuses explanation
        DrawTextLine(ctx.hdc, TextParams{ctx.fonts->uiBoldFont, L"Prestige Bonuses (per level):", Colors::Foreground}, POINT{cx, cy});
        cy += kSectionHeaderSpacing;
        DrawTextLine(ctx.hdc, TextParams{ctx.fonts->bodyFont, L"  +5% skill training speed", Colors::Purple}, POINT{cx, cy});
        cy += LINE_HEIGHT;
        DrawTextLine(ctx.hdc, TextParams{ctx.fonts->bodyFont, L"  +3% payout bonus", Colors::Green}, POINT{cx, cy});
        cy += LINE_HEIGHT;
        DrawTextLine(ctx.hdc, TextParams{ctx.fonts->bodyFont, L"  Cash converts at 10:1 ratio", Colors::Orange}, POINT{cx, cy});
        cy += LINE_HEIGHT;

        auto plvl = std::make_unique<wchar_t[]>(BUF_MEDIUM);
        StringCchPrintfW(plvl.get(), BUF_MEDIUM, L"Current Prestige Level: %d", gs->prestigeLevel);
        DrawTextLine(ctx.hdc, TextParams{ctx.fonts->bodyFont, plvl.get(), Colors::Pink}, POINT{cx, cy + kBonusTextOffsetY});

    }
    else
    {
        DrawTextLine(ctx.hdc, TextParams{ctx.fonts->uiBoldFont, L"LEGITIMATE CYBERSECURITY FIRM", Colors::Green}, POINT{cx, cy});
        cy += kSectionHeaderSpacing;
        DrawTextLine(ctx.hdc, TextParams{ctx.fonts->bodyFont, L"You're now running a legitimate operation.", Colors::Comment}, POINT{cx + kTextIndent, cy});
        cy += LINE_HEIGHT;

        auto plvl = std::make_unique<wchar_t[]>(BUF_MEDIUM);
        StringCchPrintfW(plvl.get(), BUF_MEDIUM, L"Company Valuation Tier: %d", gs->prestigeLevel);
        DrawTextLine(ctx.hdc, TextParams{ctx.fonts->bodyFont, plvl.get(), Colors::Pink}, POINT{cx + kTextIndent, cy});
        cy += LINE_HEIGHT * 2;

        DrawTextLine(ctx.hdc, TextParams{ctx.fonts->bodyFont, L"Check the Jobs panel for legitimate contracts.", Colors::Cyan}, POINT{cx + kTextIndent, cy});
    }
}

// ============================================================================
// Toast Notifications
// ============================================================================

void DrawToasts(const DrawCtx& ctx, const GameState* gs, WindowSize ws)
{
    constexpr int kToastMargin = 20;
    constexpr int kMaxToastWidth = 500;
    constexpr int kMinToastWidth = 250;
    constexpr int kToastPaddingH = 24;
    constexpr int kToastPaddingV = 10;
    constexpr int kToastMinHeight = 24;
    constexpr int kToastGap = 4;
    constexpr int kAccentWidth = 3;
    constexpr int kTextLeftPad = 10;
    constexpr int kTextTopPad = 5;
    constexpr int kTextRightPad = 8;
    constexpr int kTextBottomPad = 5;
    constexpr int kMeasurePad = 20;

    int toastY = ws.height - STATUSBAR_HEIGHT - kToastMargin;

    for (int i = MAX_TOASTS - 1; i >= 0; i--)
    {
        if (false == gs->toasts[i].active)
        {
            continue;
        }

        // Measure text width to auto-size the toast
        HFONT oldFont = static_cast<HFONT>(SelectObject(ctx.hdc, ctx.fonts->smallFont));
        RECT measureRC = { 0, 0, kMaxToastWidth - kMeasurePad, 0 };
        DrawTextW(ctx.hdc, gs->toasts[i].text, -1, &measureRC, DT_CALCRECT | DT_WORDBREAK | DT_NOPREFIX);

        int textW = measureRC.right - measureRC.left;
        int textH = measureRC.bottom - measureRC.top;

        int toastW = textW + kToastPaddingH;
        if (toastW < kMinToastWidth)
        {
            toastW = kMinToastWidth;
        }
        if (toastW > kMaxToastWidth)
        {
            toastW = kMaxToastWidth;
        }

        int toastH = textH + kToastPaddingV;
        if (toastH < kToastMinHeight)
        {
            toastH = kToastMinHeight;
        }

        int toastX = ws.width - toastW - kToastMargin;
        toastY -= toastH + kToastGap;

        DrawFilledRect(ctx.hdc, RECT{toastX, toastY, toastX + toastW, toastY + toastH}, Colors::BackgroundSecondary);
        // Left accent line
        DrawFilledRect(ctx.hdc, RECT{toastX, toastY, toastX + kAccentWidth, toastY + toastH}, gs->toasts[i].color);

        // Draw text with word wrap
        SetTextColor(ctx.hdc, gs->toasts[i].color);
        SetBkMode(ctx.hdc, TRANSPARENT);
        RECT textRC = { toastX + kTextLeftPad, toastY + kTextTopPad, toastX + toastW - kTextRightPad, toastY + toastH - kTextBottomPad };
        DrawTextW(ctx.hdc, gs->toasts[i].text, -1, &textRC, DT_WORDBREAK | DT_NOPREFIX);

        SelectObject(ctx.hdc, oldFont);
    }
}

// ============================================================================
// Hit Testing
// ============================================================================

HitResult HitTest(const GameState* gs, POINT mouse, WindowSize ws)
{
    constexpr int kTabStartY = 50;
    constexpr int kFilterHitHeight = 20;
    constexpr int kFilterHitWidth = 45;
    constexpr int kFilterTabWidth = 50;
    constexpr int kFilterCount = 5;
    constexpr int kFilterAfterSpacing = 32;
    constexpr int kBossCardSpacing = 90;
    constexpr int kBossPrepButtonStartX = 180;
    constexpr int kBossPrepButtonSpacing = 110;
    constexpr int kBossPrepButtonWidth = 100;
    constexpr int kBossPrepButtonTop = 38;
    constexpr int kBossPrepButtonBottom = 56;
    constexpr int kBossHackButtonWidth = 100;
    constexpr int kBossHackButtonOffsetX = 110;
    constexpr int kBossHackButtonTop = 58;
    constexpr int kBossHackButtonBottom = 80;
    constexpr int kJobCardSpacing = 66;
    constexpr int kJobMinHeight = 65;
    constexpr int kJobMinSpace = 20;
    constexpr int kStartButtonWidth = 70;
    constexpr int kStartButtonOffsetX = 80;
    constexpr int kStartButtonTop = 8;
    constexpr int kStartButtonBottom = 32;
    constexpr int kTitleSpacing = 36;
    constexpr int kSeparatorSpacing = 8;
    constexpr int kSectionHeaderSpacing = 24;
    constexpr int kSlotInfoSpacing = 24;
    constexpr int kAfterSlotButton = 28;
    constexpr int kCardSpacing = 48;
    constexpr int kEmptyPersonnelSpacing = 24;
    constexpr int kHireSpacing = 12;
    constexpr int kHireEntryHeight = 26;
    constexpr int kHireMinSpace = 20;
    constexpr int kBuyButtonWidth = 60;
    constexpr int kBuyButtonHeight = 20;
    constexpr int kBuyButtonOffsetX = 70;
    constexpr int kSkillRowHeight = 26;
    constexpr int kColumnHeaderSpacing = 22;
    constexpr int kDocRowHeight = 26;
    constexpr int kDocMinSpace = 20;
    constexpr int kDocMinHeight = 28;
    constexpr int kStudyButtonWidth = 60;
    constexpr int kStudyButtonOffsetX = 80;
    constexpr int kStudyButtonTop = 2;
    constexpr int kStudyButtonBottom = 20;
    constexpr int kDocSectionSpacing = 8;
    constexpr int kMarketItemSpacing = 34;
    constexpr int kMarketMinSpace = 20;
    constexpr int kMarketBuyButtonWidth = 50;
    constexpr int kMarketBuyButtonOffsetX = 60;
    constexpr int kMarketBuyButtonTop = 4;
    constexpr int kMarketBuyButtonBottom = 26;
    constexpr int kIntelStashHeaderSpacing = 20;
    constexpr int kIntelCategorySpacing = 4;
    constexpr int kIntelCategoryHeaderHeight = 22;
    constexpr int kIntelBuyButtonX = 540;
    constexpr int kIntelBuyButtonWidth = 50;
    constexpr int kIntelBuyButtonHeight = 18;
    constexpr int kResetButtonWidth = 130;
    constexpr int kResetButtonHeight = 24;
    constexpr int kResetButtonBottomOffset = 40;
    constexpr int kResetButtonIndent = 4;
    constexpr int kPrestigeButtonWidth = 200;
    constexpr int kPrestigeButtonHeight = 32;
    constexpr int kPrestigeButtonIndent = 4;

    HitResult result = { HitResult::None, 0, 0 };

    int mouseX = mouse.x;
    int mouseY = mouse.y;
    int windowWidth = ws.width;
    int windowHeight = ws.height;

    // Sidebar tabs
    if (mouseX < SIDEBAR_WIDTH)
    {
        int tabY = kTabStartY;
        for (int i = 0; i < static_cast<int>(Tab::COUNT); i++)
        {
            // Hide Military tab until available
            if (static_cast<Tab>(i) == Tab::Military &&
                false == gs->militaryEnlistAvailable &&
                MilitaryPhase::None == gs->militaryPhase &&
                false == gs->cyberVeteran)
            {
                continue;
            }

            // Hide Prestige tab until requirements met
            if (static_cast<Tab>(i) == Tab::Prestige && false == ArePrestigeRequirementsMet(gs) && false == gs->inPrestigeMode)
            {
                continue;
            }

            if (mouseY >= tabY && mouseY < tabY + TAB_HEIGHT)
            {
                result.type = HitResult::SidebarTab;
                result.index = i;
                return result;
            }
            tabY += TAB_HEIGHT;
        }
        return result;
    }

    int panelX = SIDEBAR_WIDTH;
    int panelY = 0;
    int panelW = windowWidth - SIDEBAR_WIDTH;
    int panelH = windowHeight - STATUSBAR_HEIGHT;

    // Dashboard panel hit testing
    if (Tab::Dashboard == gs->currentTab)
    {
        int cx = panelX + PADDING;
        int resetY = panelH - kResetButtonBottomOffset;
        if (mouseX >= cx + kResetButtonIndent && mouseX < cx + kResetButtonIndent + kResetButtonWidth && mouseY >= resetY && mouseY < resetY + kResetButtonHeight)
        {
            result.type = HitResult::ResetSave;
            return result;
        }
    }

    // Jobs panel hit testing
    if (Tab::Jobs == gs->currentTab)
    {
        int cx = panelX + PADDING;
        int cy = panelY + PADDING + kTitleSpacing; // After title

        // Filter tabs -- only hit-test visible (unlocked) tiers
        bool filterVisible[] = { true, true, gs->tierBossCompleted[0], gs->tierBossCompleted[1], gs->tierBossCompleted[2] };
        if (mouseY >= cy && mouseY < cy + kFilterHitHeight)
        {
            int filterX = cx;
            for (int i = 0; i < kFilterCount; i++)
            {
                if (false == filterVisible[i])
                {
                    continue;
                }
                if (mouseX >= filterX && mouseX < filterX + kFilterHitWidth)
                {
                    result.type = HitResult::JobFilter;
                    result.index = i;
                    return result;
                }
                filterX += kFilterTabWidth;
            }
        }

        cy += kSectionHeaderSpacing + kSeparatorSpacing; // After filter + separator

        // Boss hack hit testing
        for (int b = 0; b < NUM_BOSS_HACKS; b++)
        {
            if (true == gs->tierBossCompleted[b])
            {
                continue;
            }
            int requiredRep = GetBossRequiredRep(b);
            if (gs->reputation < requiredRep)
            {
                continue;
            }

            // Prep buttons
            for (int p = 0; p < MAX_BOSS_PREP; p++)
            {
                if (true == gs->bossPrepDone[b][p]) { continue; }
                int px = cx + kBossPrepButtonStartX + p * kBossPrepButtonSpacing;
                if (mouseX < px || mouseX >= px + kBossPrepButtonWidth) { continue; }
                if (mouseY < cy + kBossPrepButtonTop || mouseY >= cy + kBossPrepButtonBottom) { continue; }
                if (gs->cash < GetBossHackDef(b).prep[p].cost) { continue; }
                result.type = HitResult::BossPrep;
                result.index = b;
                result.subIndex = p;
                return result;
            }

            // HACK button
            if (gs->bossAttemptCooldown[b] <= 0)
            {
                int btnX = cx + panelW - PADDING * 2 - kBossHackButtonOffsetX;
                if (mouseX >= btnX && mouseX < btnX + kBossHackButtonWidth && mouseY >= cy + kBossHackButtonTop && mouseY < cy + kBossHackButtonBottom)
                {
                    result.type = HitResult::BossAttempt;
                    result.index = b;
                    return result;
                }
            }

            cy += kBossCardSpacing;
        }

        // Job entries
        int jobCount = GetJobCount();
        int visibleJobs = 0;
        for (int i = 0; i < jobCount; i++)
        {
            const JobDef& job = GetJobDef(i);

            if (gs->jobFilterTier > 0)
            {
                int filterTier = kFilterCount - gs->jobFilterTier;
                if (static_cast<int>(job.tier) != filterTier)
                {
                    continue;
                }
            }
            // Hide locked tiers
            if (Tier::Three == job.tier && false == gs->tierBossCompleted[0])
            {
                continue;
            }
            if (Tier::Two == job.tier && false == gs->tierBossCompleted[1])
            {
                continue;
            }
            if (Tier::One == job.tier && false == gs->tierBossCompleted[2])
            {
                continue;
            }

            if (true == job.isLegitimate && false == gs->inPrestigeMode)
            {
                continue;
            }
            if (false == job.isLegitimate && true == gs->inPrestigeMode)
            {
                continue;
            }

            if (visibleJobs < gs->jobScrollOffset)
            {
                visibleJobs++;
                continue;
            }

            if (cy + kJobMinHeight > panelY + panelH - kJobMinSpace)
            {
                break;
            }

            // Check START button click
            {
                int btnX = cx + panelW - PADDING * 2 - kStartButtonOffsetX;
                bool mouseInButton = mouseY >= cy + kStartButtonTop && mouseY < cy + kStartButtonBottom && mouseX >= btnX && mouseX < btnX + kStartButtonWidth;
                if (mouseInButton && true == CanStartJob(gs, i))
                {
                    result.type = HitResult::JobStart;
                    result.index = i;
                    return result;
                }
            }

            cy += kJobCardSpacing;
            visibleJobs++;
        }
    }

    // Personnel panel
    if (Tab::Personnel == gs->currentTab)
    {
        int cx = panelX + PADDING;
        int cy = panelY + PADDING + kTitleSpacing + kSlotInfoSpacing + kSeparatorSpacing - gs->personnelScrollOffset * LINE_HEIGHT; // After title, slots, separator (with scroll)

        // Buy Personnel Slot hit test
        if (GetMaxPersonnelSlots(gs) < MAX_PERSONNEL)
        {
            cy += kSectionHeaderSpacing; // After "EXPAND PERSONNEL CAPACITY" header

            int64_t slotCost = GetNextPersonnelSlotCost(gs);
            bool canAffordSlot = gs->cash >= slotCost;
            int btnX = cx + panelW - PADDING * 2 - kBuyButtonOffsetX;
            bool mouseInButton = true == canAffordSlot && mouseX >= btnX && mouseX < btnX + kBuyButtonWidth && mouseY >= cy && mouseY < cy + kBuyButtonHeight;
            if (mouseInButton)
            {
                result.type = HitResult::PersonnelBuySlot;
                result.index = 0;
                return result;
            }

            cy += kAfterSlotButton + kSeparatorSpacing; // After slot button + separator
        }

        cy += kSectionHeaderSpacing; // After "ACTIVE PERSONNEL" header

        // Skip active personnel
        for (int i = 0; i < MAX_PERSONNEL; i++)
        {
            if (false == gs->personnel[i].active)
            {
                continue;
            }
            cy += kCardSpacing;
        }

        int personnelCount = 0;
        for (int i = 0; i < MAX_PERSONNEL; i++)
        {
            if (true == gs->personnel[i].active)
            {
                personnelCount++;
            }
        }
        if (0 == personnelCount)
        {
            cy += kEmptyPersonnelSpacing;
        }

        cy += kHireSpacing + kSeparatorSpacing + kSectionHeaderSpacing; // spacing + separator + header

        // Hire buttons
        for (int i = 0; i < static_cast<int>(PersonnelType::COUNT); i++)
        {
            if (cy + 30 > panelY + panelH - kHireMinSpace) { break; }

            const PersonnelTypeDef& pt = g_PersonnelTypes[i];
            bool repMet = gs->reputation >= pt.requiredRep;
            if (false == repMet) { cy += kHireEntryHeight; continue; }

            int btnX = cx + panelW - PADDING * 2 - kBuyButtonOffsetX;
            if (mouseX >= btnX && mouseX < btnX + kBuyButtonWidth && mouseY >= cy && mouseY < cy + kBuyButtonHeight)
            {
                result.type = HitResult::PersonnelHire;
                result.index = i;
                return result;
            }
            cy += kHireEntryHeight;
        }
    }

    // Skills panel - documentation study buttons
    if (Tab::Skills == gs->currentTab)
    {
        int cx = panelX + PADDING;
        int scrollPixels = gs->skillScrollOffset * kSkillRowHeight;
        int cy = panelY + PADDING + kTitleSpacing + kSeparatorSpacing + kColumnHeaderSpacing - scrollPixels; // After title, separator, column headers (with scroll)

        // Skip skill rows
        for (int i = 0; i < NUM_SKILLS; i++)
        {
            cy += kSkillRowHeight;
        }

        // Documentation section
        cy += kDocSectionSpacing + kDocSectionSpacing + kSectionHeaderSpacing; // separator spacing + separator + header

        int docCount = GetDocCount();
        for (int i = 0; i < docCount; i++)
        {
            if (cy + kDocMinHeight > panelY + panelH - kDocMinSpace)
            {
                break;
            }

            bool isStudying = (true == gs->docStudyActive && static_cast<int>(gs->currentDocStudy) == i);
            int passes = gs->docStudyPasses[i];
            bool canStudyMore = (passes < MAX_DOC_PASSES);

            if (false == canStudyMore || true == isStudying) { cy += kDocRowHeight; continue; }

            const DocStudyDef& doc = GetDocDef(i);
            // First pass costs money, subsequent passes are free
            int64_t cost = (0 == passes) ? doc.cost : 0;
            bool canStudy = false == gs->docStudyActive && (0 == cost || gs->cash >= cost);
            if (false == canStudy) { cy += kDocRowHeight; continue; }

            int btnX = cx + panelW - PADDING * 2 - kStudyButtonOffsetX;
            if (mouseX >= btnX && mouseX < btnX + kStudyButtonWidth && mouseY >= cy + kStudyButtonTop && mouseY < cy + kStudyButtonBottom)
            {
                result.type = HitResult::DocStudy;
                result.index = i;
                return result;
            }

            cy += kDocRowHeight;
        }
    }

    // Market panel - buy buttons
    if (Tab::Market == gs->currentTab)
    {
        int cx = panelX + PADDING;
        int cy = panelY + PADDING + kTitleSpacing + kSeparatorSpacing + kSectionHeaderSpacing - gs->marketScrollOffset * LINE_HEIGHT; // After title, separator, header (with scroll)

        for (int i = 0; i < g_NumMarketItems; i++)
        {
            if (cy + kMarketItemSpacing > panelY + panelH - kMarketMinSpace)
            {
                break;
            }

            const MarketItem& item = g_MarketItems[i];
            int btnX = cx + panelW - PADDING * 2 - kMarketBuyButtonOffsetX;

            // Sell items: only present when owned (kept in sync with DrawMarketPanel)
            if (false == item.isBuyable)
            {
                if (MarketSellableCount(gs, item.type) <= 0) { continue; }
                if (mouseX >= btnX && mouseX < btnX + kMarketBuyButtonWidth && mouseY >= cy + kMarketBuyButtonTop && mouseY < cy + kMarketBuyButtonBottom)
                {
                    result.type = HitResult::MarketSell;
                    result.index = i;
                    return result;
                }
                cy += kMarketItemSpacing;
                continue;
            }

            int64_t price = static_cast<int64_t>(item.basePrice * gs->market.priceMultipliers[static_cast<int>(item.type)]);
            bool canAfford = gs->cash >= price;

            if (false == canAfford) { cy += kMarketItemSpacing; continue; }

            if (mouseX >= btnX && mouseX < btnX + kMarketBuyButtonWidth && mouseY >= cy + kMarketBuyButtonTop && mouseY < cy + kMarketBuyButtonBottom)
            {
                result.type = HitResult::MarketBuy;
                result.index = i;
                return result;
            }
            cy += kMarketItemSpacing;
        }
    }

    // Intel panel - buy buttons
    if (Tab::Intel == gs->currentTab)
    {
        int cx = panelX + PADDING;
        int cy = panelY + PADDING + kTitleSpacing + LINE_HEIGHT + kSeparatorSpacing; // After title and subtitle

        // Skip stash section if any intel owned
        bool hasIntel = false;
        for (int i = 0; i < static_cast<int>(IntelItemType::COUNT); i++)
        {
            if (gs->intelInventory[i] > 0)
            {
                hasIntel = true;
                break;
            }
        }
        if (true == hasIntel)
        {
            cy += kIntelStashHeaderSpacing; // "YOUR STASH:" header
            for (int i = 0; i < static_cast<int>(IntelItemType::COUNT); i++)
            {
                if (gs->intelInventory[i] > 0)
                {
                    cy += LINE_HEIGHT;
                }
            }
            cy += kSeparatorSpacing + 1 + kSeparatorSpacing; // spacing + separator + spacing
        }

        // Apply scroll offset
        cy -= gs->intelScrollOffset * LINE_HEIGHT;

        // Walk through items matching DrawIntelPanel layout
        const wchar_t* lastCategory = L"";
        for (int i = 0; i < static_cast<int>(IntelItemType::COUNT); i++)
        {
            const IntelItemDef& item = g_IntelItems[i];

            if (0 != lstrcmpW(item.category, lastCategory))
            {
                lastCategory = item.category;
                cy += kIntelCategorySpacing + kIntelCategoryHeaderHeight; // category header
            }

            bool repOk = gs->reputation >= item.repRequired;
            if (false == repOk)
            {
                cy += LINE_HEIGHT; // locked item
                continue;
            }

            bool alreadyOwned = gs->intelInventory[i] > 0;
            if (false == alreadyOwned)
            {
                int64_t price = static_cast<int64_t>(item.basePrice * gs->intelPriceMultipliers[i]);
                bool canAfford = gs->cash >= price;
                int stock = gs->intelMarketStock[i];

                int btnX = cx + kIntelBuyButtonX;
                bool mouseInButton = stock > 0 && true == canAfford && mouseX >= btnX && mouseX < btnX + kIntelBuyButtonWidth && mouseY >= cy && mouseY < cy + kIntelBuyButtonHeight;
                if (mouseInButton)
                {
                    result.type = HitResult::IntelBuy;
                    result.index = i;
                    return result;
                }
            }
            cy += LINE_HEIGHT; // item row
            cy += LINE_HEIGHT; // description row
        }
    }

    // Military panel
    if (Tab::Military == gs->currentTab)
    {
        int cx = panelX + PADDING;

        if (MilitaryPhase::None == gs->militaryPhase && true == gs->militaryEnlistAvailable && false == gs->cyberVeteran)
        {
            // Enlist button position (approximate from DrawMilitaryPanel layout)
            constexpr int kEnlistButtonWidth = 300;
            constexpr int kEnlistButtonHeight = 36;
            constexpr int kTextIndent = 4;
            int cy = panelY + PADDING + 36 + 8 - gs->militaryScrollOffset * LINE_HEIGHT; // title + separator (with scroll)
            cy += 24 + LINE_HEIGHT * 2 + LINE_HEIGHT * 2; // header + description
            cy += 24 + LINE_HEIGHT * 4 + LINE_HEIGHT * 2; // terms
            cy += 24 + LINE_HEIGHT * 3 + LINE_HEIGHT * 2; // completion
            cy += 24 + LINE_HEIGHT * 2 + LINE_HEIGHT * 2; // warning

            if (mouseX >= cx + kTextIndent && mouseX < cx + kTextIndent + kEnlistButtonWidth &&
                mouseY >= cy && mouseY < cy + kEnlistButtonHeight)
            {
                result.type = HitResult::MilitaryEnlist;
                return result;
            }
        }
        else if (MilitaryPhase::SpecialtyPick == gs->militaryPhase)
        {
            // Specialty skill buttons
            constexpr int kSkillButtonWidth = 200;
            constexpr int kSkillButtonHeight = 28;
            constexpr int kSkillButtonsPerRow = 3;
            constexpr int kSkillButtonSpacingX = 210;
            constexpr int kTextIndent = 4;

            int cy = panelY + PADDING + 36 + 8 + 24 + LINE_HEIGHT * 3 - gs->militaryScrollOffset * LINE_HEIGHT; // header area (with scroll)

            for (int i = 0; i < NUM_SKILLS; i++)
            {
                int col = i % kSkillButtonsPerRow;
                int row = i / kSkillButtonsPerRow;
                int btnX = cx + kTextIndent + col * kSkillButtonSpacingX;
                int btnY = cy + row * (kSkillButtonHeight + 6);

                if (mouseX >= btnX && mouseX < btnX + kSkillButtonWidth &&
                    mouseY >= btnY && mouseY < btnY + kSkillButtonHeight)
                {
                    result.type = HitResult::MilitarySpecialtyPick;
                    result.index = i;
                    return result;
                }
            }
        }
    }

    // Prestige panel
    if (Tab::Prestige == gs->currentTab && false == gs->inPrestigeMode)
    {
        int cx = panelX + PADDING;
        // Approximate button position
        int cy = panelY + PADDING + kTitleSpacing + kSeparatorSpacing + kSectionHeaderSpacing + kSectionHeaderSpacing + LINE_HEIGHT * 2 + kSectionHeaderSpacing + LINE_HEIGHT * 3 + LINE_HEIGHT * 2;
        if (mouseX >= cx + kPrestigeButtonIndent && mouseX < cx + kPrestigeButtonIndent + kPrestigeButtonWidth && mouseY >= cy && mouseY < cy + kPrestigeButtonHeight)
        {
            result.type = HitResult::PrestigeActivate;
            return result;
        }
    }

    return result;
}
