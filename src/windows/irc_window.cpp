#include "irc_window.h"
#include "../ui/colors.h"
#include "../data/strings.h"
#include <strsafe.h>
#include <memory>

// ============================================================================
// IRC Window State
// ============================================================================

HWND g_IRCWnd = NULL;
static HFONT g_IRCFont = NULL;

static std::vector<IRCMessage> g_IRCMessages;
static int g_IRCCurrentLine = 0;
static int g_IRCDelayAccumulator = 0;
static bool g_IRCIsScam = false;
static wchar_t g_IRCChannel[BUF_MEDIUM] = {};

// Pointer to the game state (set during spawn calls)
static GameState* s_pGameState = nullptr;

// ============================================================================
// Dynamic IRC Message Builder
// ============================================================================

static void BuildIRCMessages_Dynamic(std::vector<IRCMessage>& msgs, int typeIndex, bool isSuccess, const wchar_t* handle, GameState* pGameState)
{
    auto add = [&](const wchar_t* sender, const wchar_t* text, COLORREF color, int delay)
    {
        IRCMessage m = {};
        StringCchCopyW(m.sender, BUF_SMALL, sender);
        StringCchCopyW(m.text, BUF_XLARGE, text);
        m.color = color;
        m.delayMs = delay;
        msgs.push_back(m);
    };

    PersonnelType type = static_cast<PersonnelType>(typeIndex);

    // Pick a random second person in the channel (bystander)
    int bystanderIdx = (typeIndex * 17 + static_cast<int>(pGameState->totalTicksPlayed % 500)) % NUM_HACKER_USERNAMES;
    if (bystanderIdx < 0)
    {
        bystanderIdx = -bystanderIdx;
    }
    const wchar_t* bystander = g_HackerUsernames[bystanderIdx];

    if (true == isSuccess)
    {
        // Join message
        auto joinBuf = std::make_unique<wchar_t[]>(BUF_XLARGE);
        StringCchPrintfW(joinBuf.get(), BUF_XLARGE, L"%s has joined the channel", handle);
        add(L"*", joinBuf.get(), Colors::Purple, 500);

        // Greeting - pick from pool
        int greetIdx = static_cast<int>(pGameState->totalTicksPlayed % NUM_IRC_GREETINGS_SUCCESS);
        add(handle, g_IRCGreetingsSuccess[greetIdx], Colors::Cyan, 1200);

        // Specialty line - find one for this type
        int specCount = 0;
        constexpr int kMaxSpecIndices = 8;
        int specIndices[kMaxSpecIndices] = {};
        for (int i = 0; i < NUM_IRC_SPECIALTY_LINES && specCount < kMaxSpecIndices; i++)
        {
            if (g_IRCSpecialtyLines[i].type == type)
            {
                specIndices[specCount++] = i;
            }
        }
        if (specCount > 0)
        {
            int pick = static_cast<int>((pGameState->totalTicksPlayed / 3) % specCount);
            add(handle, g_IRCSpecialtyLines[specIndices[pick]].text, Colors::Cyan, 1800);
        }

        // Bystander reacts
        add(bystander, L"vouch. they're legit", Colors::Comment, 1400);

        // Banter line
        int banterIdx = static_cast<int>((pGameState->totalTicksPlayed / 7) % NUM_IRC_BANTER_LINES);
        add(handle, g_IRCBanterLines[banterIdx], Colors::Cyan, 1500);

        // Outcome
        int outcomeIdx = static_cast<int>((pGameState->totalTicksPlayed / 11) % NUM_IRC_OUTCOME_SUCCESS);
        add(handle, g_IRCOutcomeSuccess[outcomeIdx], Colors::Green, 1500);

        // Join message
        auto joinMsg = std::make_unique<wchar_t[]>(BUF_XLARGE);
        StringCchPrintfW(joinMsg.get(), BUF_XLARGE, L"%s has joined your crew", handle);
        add(L"*", joinMsg.get(), Colors::Purple, 800);
    }
    else
    {
        // Scam flow
        auto joinBuf = std::make_unique<wchar_t[]>(BUF_XLARGE);
        StringCchPrintfW(joinBuf.get(), BUF_XLARGE, L"%s has joined the channel", handle);
        add(L"*", joinBuf.get(), Colors::Purple, 500);

        int greetIdx = static_cast<int>(pGameState->totalTicksPlayed % NUM_IRC_GREETINGS_SCAM);
        add(handle, g_IRCGreetingsScam[greetIdx], Colors::Cyan, 1200);

        // Suspicious bystander warning
        add(bystander, L"careful... never seen this guy before", Colors::Orange, 2000);

        add(handle, L"ignore him. he's just jealous. so we doing this or what?", Colors::Cyan, 1500);

        // Scam outcome
        int outcomeIdx = static_cast<int>((pGameState->totalTicksPlayed / 5) % NUM_IRC_OUTCOME_SCAM);
        add(handle, g_IRCOutcomeScam[outcomeIdx], Colors::Red, 2500);

        add(bystander, L"called it. rip your wallet", Colors::Orange, 1200);

        auto leaveMsg = std::make_unique<wchar_t[]>(BUF_XLARGE);
        StringCchPrintfW(leaveMsg.get(), BUF_XLARGE, L"%s has left the channel", handle);
        add(L"*", leaveMsg.get(), Colors::Purple, 500);
    }
}

// ============================================================================
// IRC Job Unlock Dialogue Builder
// ============================================================================

static void BuildIRCMessages_JobUnlock(std::vector<IRCMessage>& msgs, int jobDefId, GameState* pGameState)
{
    auto add = [&](const wchar_t* sender, const wchar_t* text, COLORREF color, int delay)
    {
        IRCMessage m = {};
        StringCchCopyW(m.sender, BUF_SMALL, sender);
        StringCchCopyW(m.text, BUF_XLARGE, text);
        m.color = color;
        m.delayMs = delay;
        msgs.push_back(m);
    };

    const JobDef& job = GetJobDef(jobDefId);

    int nameIdx = (jobDefId * 31 + static_cast<int>(pGameState->totalTicksPlayed % 1000)) % NUM_HACKER_USERNAMES;
    if (nameIdx < 0)
    {
        nameIdx = -nameIdx;
    }
    const wchar_t* handle = g_HackerUsernames[nameIdx];

    add(L"*", L"-- You've been noticed in the scene --", Colors::Comment, 500);
    add(handle, L"yo, saw you grinding on the boards", Colors::Cyan, 1200);
    add(handle, L"got something you might be interested in", Colors::Cyan, 1500);

    auto tipBuf = std::make_unique<wchar_t[]>(BUF_XLARGE);
    StringCchPrintfW(tipBuf.get(), BUF_XLARGE, L"there's a gig called \"%s\"", job.name);
    add(handle, tipBuf.get(), Colors::Green, 1800);

    if (job.numSkillsTrained > 0)
    {
        const wchar_t* skillName = SkillNames[job.skillsTrained[0].skill];
        auto skillBuf = std::make_unique<wchar_t[]>(BUF_XLARGE);
        StringCchPrintfW(skillBuf.get(), BUF_XLARGE, L"you'll want to know your %s for this one", skillName);
        add(handle, skillBuf.get(), Colors::Yellow, 1500);
    }

    constexpr int64_t kHighPayoutThreshold = 1000;
    auto payBuf = std::make_unique<wchar_t[]>(BUF_XLARGE);
    if (job.basePayoutUsd >= kHighPayoutThreshold)
    {
        StringCchPrintfW(payBuf.get(), BUF_XLARGE, L"pays decent too. we're talking $%lld+ per run", job.basePayoutUsd);
    }
    else
    {
        StringCchPrintfW(payBuf.get(), BUF_XLARGE, L"doesn't pay much ($%lld) but it'll get you cred", job.basePayoutUsd);
    }
    add(handle, payBuf.get(), Colors::Green, 1500);

    constexpr int kHeatWarningThreshold = 5;
    if (job.heatGenerated >= kHeatWarningThreshold)
    {
        add(handle, L"careful tho. feds are watching that kind of thing", Colors::Red, 1200);
    }

    add(handle, L"check your Jobs panel. good luck out there", Colors::Cyan, 1500);
    add(L"*", L"-- Session ended --", Colors::Comment, 800);
}

// ============================================================================
// IRC Window Spawning (Job Unlock)
// ============================================================================

void SpawnJobUnlockIRCWindow(GameState* pGameState, HWND hMainWnd, int jobDefId)
{
    if (nullptr != g_IRCWnd)
    {
        DestroyWindow(g_IRCWnd);
        g_IRCWnd = NULL;
    }

    s_pGameState = pGameState;
    g_IRCMessages.clear();
    g_IRCCurrentLine = 0;
    g_IRCDelayAccumulator = 0;
    g_IRCIsScam = false;

    BuildIRCMessages_JobUnlock(g_IRCMessages, jobDefId, pGameState);

    if (true == g_IRCMessages.empty())
    {
        return;
    }

    wchar_t title[BUF_LARGE] = {};
    StringCchPrintfW(title, BUF_LARGE, L"IRC - #underground-tips");

    HINSTANCE hInst = reinterpret_cast<HINSTANCE>(GetWindowLongPtrW(hMainWnd, GWLP_HINSTANCE));

    constexpr int kIRCUnlockWidth = 540;
    constexpr int kIRCUnlockHeight = 380;
    constexpr int kIRCUnlockOffset = 10;

    RECT mainRC;
    GetWindowRect(hMainWnd, &mainRC);
    int ircX = mainRC.right + kIRCUnlockOffset;
    int ircY = mainRC.top + 50;

    int screenW = GetSystemMetrics(SM_CXSCREEN);
    if (ircX + kIRCUnlockWidth > screenW)
    {
        ircX = mainRC.left - kIRCUnlockWidth - kIRCUnlockOffset;
        if (ircX < 0) ircX = 50;
    }

    g_IRCWnd = CreateWindowExW(
        WS_EX_TOOLWINDOW,
        L"ShadowNetIRCClass",
        title,
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU,
        ircX, ircY,
        kIRCUnlockWidth, kIRCUnlockHeight,
        hMainWnd,
        NULL,
        hInst,
        NULL);

    if (nullptr != g_IRCWnd)
    {
        ShowWindow(g_IRCWnd, SW_SHOW);
        UpdateWindow(g_IRCWnd);
        SetTimer(g_IRCWnd, TIMER_IRC, IRC_TICK_MS, NULL);
    }
}

// ============================================================================
// IRC Window Spawning (Personnel Hiring)
// ============================================================================

void SpawnIRCWindow(GameState* pGameState, HWND hMainWnd, int typeIndex, bool isSuccess, const wchar_t* personnelHandle)
{
    if (nullptr != g_IRCWnd)
    {
        DestroyWindow(g_IRCWnd);
        g_IRCWnd = NULL;
    }

    s_pGameState = pGameState;
    g_IRCMessages.clear();
    g_IRCCurrentLine = 0;
    g_IRCDelayAccumulator = 0;
    g_IRCIsScam = !isSuccess;

    const PersonnelTypeDef& def = g_PersonnelTypes[typeIndex];

    StringCchPrintfW(g_IRCChannel, BUF_MEDIUM, L"#darknet-%s", def.name);
    for (int i = 0; L'\0' != g_IRCChannel[i]; i++)
    {
        if (L' ' == g_IRCChannel[i])
        {
            g_IRCChannel[i] = L'-';
        }
        else
        {
            g_IRCChannel[i] = towlower(g_IRCChannel[i]);
        }
    }

    BuildIRCMessages_Dynamic(g_IRCMessages, typeIndex, isSuccess, personnelHandle, pGameState);

    if (true == g_IRCMessages.empty())
    {
        return;
    }

    wchar_t title[BUF_LARGE] = {};
    StringCchPrintfW(title, BUF_LARGE, L"IRC - %s", g_IRCChannel);

    RECT mainRC;
    GetWindowRect(hMainWnd, &mainRC);
    constexpr int kIRCWindowOffset = 10;
    constexpr int kIRCWindowWidth = 540;
    constexpr int kIRCWindowHeight = 400;
    constexpr int kIRCFallbackX = 50;
    int ircX = mainRC.right + kIRCWindowOffset;
    int ircY = mainRC.top + 50;

    int screenW = GetSystemMetrics(SM_CXSCREEN);
    if (ircX + kIRCWindowWidth > screenW)
    {
        ircX = mainRC.left - kIRCWindowWidth - kIRCWindowOffset;
    }
    if (ircX < 0)
    {
        ircX = kIRCFallbackX;
    }

    HINSTANCE hInst = reinterpret_cast<HINSTANCE>(GetWindowLongPtrW(hMainWnd, GWLP_HINSTANCE));

    g_IRCWnd = CreateWindowExW(
        WS_EX_TOOLWINDOW,
        L"ShadowNetIRCClass",
        title,
        WS_POPUP | WS_CAPTION | WS_SYSMENU,
        ircX, ircY, kIRCWindowWidth, kIRCWindowHeight,
        hMainWnd,
        NULL,
        hInst,
        NULL);

    if (nullptr == g_IRCWnd)
    {
        return;
    }

    if (nullptr != g_IRCFont)
    {
        DeleteObject(g_IRCFont);
    }
    g_IRCFont = CreateFontW(
        14, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, FIXED_PITCH | FF_MODERN, L"Consolas");

    ShowWindow(g_IRCWnd, SW_SHOW);
    UpdateWindow(g_IRCWnd);
    SetTimer(g_IRCWnd, TIMER_IRC, IRC_TICK_MS, NULL);
}

// ============================================================================
// IRC Window Procedure
// ============================================================================

LRESULT CALLBACK IRCWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
        case WM_TIMER:
        {
            if (TIMER_IRC != wParam)
            {
                return 0;
            }
            if (g_IRCCurrentLine >= static_cast<int>(g_IRCMessages.size()))
            {
                return 0;
            }

            g_IRCDelayAccumulator += IRC_TICK_MS;
            if (g_IRCDelayAccumulator >= g_IRCMessages[g_IRCCurrentLine].delayMs)
            {
                g_IRCCurrentLine++;
                g_IRCDelayAccumulator = 0;
                InvalidateRect(hWnd, NULL, FALSE);
            }
            return 0;
        }

        case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);

            RECT rc;
            GetClientRect(hWnd, &rc);
            int w = rc.right - rc.left;
            int h = rc.bottom - rc.top;

            HBRUSH bgBrush = CreateSolidBrush(Colors::BackgroundPrimary);
            FillRect(hdc, &rc, bgBrush);
            DeleteObject(bgBrush);

            HFONT oldFont = static_cast<HFONT>(SelectObject(hdc, g_IRCFont));
            SetBkMode(hdc, TRANSPARENT);

            constexpr int kHeaderHeight = 24;
            constexpr int kHeaderPadding = 8;
            constexpr int kHeaderTextY = 4;
            constexpr int kTextPadding = 8;
            constexpr int kLineHeight = 16;
            constexpr int kMessageGap = 2;
            constexpr int kTextStartY = 30;

            RECT headerRC = { 0, 0, w, kHeaderHeight };
            HBRUSH headerBrush = CreateSolidBrush(Colors::BackgroundSecondary);
            FillRect(hdc, &headerRC, headerBrush);
            DeleteObject(headerBrush);

            SetTextColor(hdc, Colors::Green);
            TextOutW(hdc, kHeaderPadding, kHeaderTextY, g_IRCChannel, static_cast<int>(wcslen(g_IRCChannel)));

            int textAreaW = w - (kTextPadding * 2);
            int totalH = 0;

            auto getWrappedHeight = [&](const wchar_t* text) -> int
            {
                RECT measureRC = { 0, 0, textAreaW, 0 };
                DrawTextW(hdc, text, -1, &measureRC, DT_CALCRECT | DT_WORDBREAK | DT_NOPREFIX);
                int measuredH = measureRC.bottom - measureRC.top;
                if (measuredH < kLineHeight)
                {
                    measuredH = kLineHeight;
                }
                return measuredH;
            };

            constexpr int kMaxFormattedTextLen = 512;
            struct FormattedMsg
            {
                wchar_t text[kMaxFormattedTextLen];
                COLORREF color;
                int height;
            };
            auto fmtMsgs = std::make_unique<FormattedMsg[]>(g_IRCCurrentLine);
            int numFmt = 0;

            for (int i = 0; i < g_IRCCurrentLine && i < static_cast<int>(g_IRCMessages.size()); i++)
            {
                const IRCMessage& m = g_IRCMessages[i];
                FormattedMsg& fm = fmtMsgs[numFmt];

                if (0 == wcscmp(m.sender, L"*"))
                {
                    StringCchPrintfW(fm.text, kMaxFormattedTextLen, L"* %s", m.text);
                    fm.color = Colors::Purple;
                }
                else if (0 == wcscmp(m.sender, L"you"))
                {
                    StringCchPrintfW(fm.text, kMaxFormattedTextLen, L"<%s> %s", m.sender, m.text);
                    fm.color = Colors::Foreground;
                }
                else
                {
                    StringCchPrintfW(fm.text, kMaxFormattedTextLen, L"<%s> %s", m.sender, m.text);
                    fm.color = m.color;
                }

                fm.height = getWrappedHeight(fm.text);
                totalH += fm.height + kMessageGap;
                numFmt++;
            }

            int visibleH = h - (kHeaderHeight + kHeaderPadding);
            int scrollOffset = 0;
            if (totalH > visibleH)
            {
                scrollOffset = totalH - visibleH;
            }

            int y = kTextStartY - scrollOffset;
            for (int i = 0; i < numFmt; i++)
            {
                FormattedMsg& fm = fmtMsgs[i];

                if (y + fm.height < kHeaderHeight)
                {
                    y += fm.height + kMessageGap;
                    continue;
                }
                if (y > h)
                {
                    break;
                }

                SetTextColor(hdc, fm.color);
                RECT textRC = { kTextPadding, y, w - kTextPadding, y + fm.height };
                DrawTextW(hdc, fm.text, -1, &textRC, DT_WORDBREAK | DT_NOPREFIX);
                y += fm.height + kMessageGap;
            }

            SelectObject(hdc, oldFont);
            EndPaint(hWnd, &ps);
            return 0;
        }

        case WM_DESTROY:
        {
            KillTimer(hWnd, TIMER_IRC);
            g_IRCWnd = NULL;
            if (nullptr != g_IRCFont)
            {
                DeleteObject(g_IRCFont);
                g_IRCFont = NULL;
            }
            return 0;
        }

        case WM_MOUSEWHEEL:
        {
            // Forward scroll to main window so intel/job scrolling still works
            HWND hParent = GetParent(hWnd);
            if (nullptr != hParent)
            {
                SendMessageW(hParent, WM_MOUSEWHEEL, wParam, lParam);
            }
            return 0;
        }
    }

    return DefWindowProcW(hWnd, msg, wParam, lParam);
}

// ============================================================================
// Cleanup
// ============================================================================

void CleanupIRCWindow()
{
    if (nullptr != g_IRCWnd)
    {
        DestroyWindow(g_IRCWnd);
        g_IRCWnd = NULL;
    }
}
