#include "boss_window.h"
#include "../ui/colors.h"
#include "../ui/ui.h"
#include <strsafe.h>
#include <memory>
#include <vector>

// ============================================================================
// Boss Window State
// ============================================================================

HWND g_BossWnd = NULL;
static HFONT g_BossFont = NULL;

static std::vector<BossLine> g_BossLines;
static int g_BossCurrentLine = 0;
static int g_BossDelayAccumulator = 0;
static int g_BossIdx = -1;
static bool g_BossSucceeded = false;

// Pointer to the game state (set during spawn)
static GameState* s_pGameState = nullptr;

// ============================================================================
// Boss Hack Execution (pre-calculate all stages)
// ============================================================================

static void ExecuteBossHack(GameState* pGameState, int bossIdx)
{
    g_BossIdx = bossIdx;
    g_BossLines.clear();
    g_BossCurrentLine = 0;
    g_BossDelayAccumulator = 0;
    g_BossSucceeded = true;

    const BossHackDef& boss = GetBossHackDef(bossIdx);

    // Calculate total prep bonus
    int prepBonus = 0;
    for (int p = 0; p < MAX_BOSS_PREP; p++)
    {
        if (true == pGameState->bossPrepDone[bossIdx][p])
        {
            prepBonus += boss.prep[p].bonusPercent;
        }
    }

    // Header line
    BossLine header = {};
    StringCchPrintfW(header.text, BUF_XLARGE, L"=== %s ===", boss.operationName);
    header.color = Colors::Red;
    header.delayMs = 1500;
    g_BossLines.push_back(header);

    BossLine target = {};
    StringCchPrintfW(target.text, BUF_XLARGE, L"Target: %s", boss.targetName);
    target.color = Colors::Orange;
    target.delayMs = 1000;
    g_BossLines.push_back(target);

    BossLine blank = {};
    StringCchPrintfW(blank.text, BUF_XLARGE, L" ");
    blank.delayMs = 500;
    g_BossLines.push_back(blank);

    constexpr int kBaseChance = 25;
    constexpr int kChancePerSkillDiff = 2;
    constexpr int kMinChance = 5;
    constexpr int kMaxChance = 90;
    constexpr int kMinRoll = 1;
    constexpr int kMaxRoll = 100;
    constexpr int kShortLineLen = 30;
    constexpr int kMedLineLen = 50;
    constexpr int kShortLineBaseDelay = 600;
    constexpr int kShortLineDelayPerChar = 20;
    constexpr int kMedLineBaseDelay = 1200;
    constexpr int kMedLineDelayPerChar = 15;
    constexpr int kLongLineBaseDelay = 1800;
    constexpr int kLongLineDelayPerChar = 10;

    for (int s = 0; s < boss.numStages; s++)
    {
        const BossStage& stage = boss.stages[s];

        BossLine stageHeader = {};
        StringCchPrintfW(stageHeader.text, BUF_XLARGE, L"--- Stage %d: %s ---", s + 1, stage.name);
        stageHeader.color = Colors::Cyan;
        stageHeader.delayMs = 1200;
        g_BossLines.push_back(stageHeader);

        for (int t = 0; t < stage.numTerminalLines; t++)
        {
            BossLine term = {};
            StringCchCopyW(term.text, BUF_XLARGE, stage.terminalLines[t]);
            term.color = Colors::Green;
            size_t len = 0;
            (void)StringCchLengthW(stage.terminalLines[t], BUF_XLARGE, &len);
            if (len < kShortLineLen)
            {
                term.delayMs = kShortLineBaseDelay + static_cast<int>(len * kShortLineDelayPerChar);
            }
            else if (len < kMedLineLen)
            {
                term.delayMs = kMedLineBaseDelay + static_cast<int>(len * kMedLineDelayPerChar);
            }
            else
            {
                term.delayMs = kLongLineBaseDelay + static_cast<int>(len * kLongLineDelayPerChar);
            }
            g_BossLines.push_back(term);
        }

        int playerSkill = pGameState->skills[stage.requiredSkill].level;
        int chance = kBaseChance + (playerSkill - stage.difficultyThreshold) * kChancePerSkillDiff + prepBonus;
        if (chance < kMinChance)
        {
            chance = kMinChance;
        }
        if (chance > kMaxChance)
        {
            chance = kMaxChance;
        }

        BossLine check = {};
        StringCchPrintfW(check.text, BUF_XLARGE, L"[SKILL CHECK] %s: %d vs DC %d  \x2014  Rolling...",
            SkillNames[stage.requiredSkill], playerSkill, stage.difficultyThreshold);
        check.color = Colors::Yellow;
        check.delayMs = 1500;
        check.isSkillCheck = true;
        g_BossLines.push_back(check);

        int roll = GameRandomRange(pGameState, kMinRoll, kMaxRoll);
        bool passed = (roll <= chance);

        BossLine result = {};
        result.isResult = true;
        result.passed = passed;
        if (true == passed)
        {
            StringCchPrintfW(result.text, BUF_XLARGE, L"\x2713 PASSED (Roll: %d%% \x2014 needed %d%%)", roll, chance);
            result.color = Colors::Green;
        }
        else
        {
            StringCchPrintfW(result.text, BUF_XLARGE, L"\x2717 FAILED (Roll: %d%% \x2014 needed %d%%)", roll, chance);
            result.color = Colors::Red;
        }
        result.delayMs = 2000;
        g_BossLines.push_back(result);

        if (false == passed)
        {
            g_BossSucceeded = false;

            BossLine fail1 = {};
            StringCchPrintfW(fail1.text, BUF_XLARGE, L" ");
            fail1.delayMs = 800;
            g_BossLines.push_back(fail1);

            BossLine fail2 = {};
            StringCchPrintfW(fail2.text, BUF_XLARGE, L"!!! CONNECTION LOST !!!");
            fail2.color = Colors::Red;
            fail2.delayMs = 1500;
            g_BossLines.push_back(fail2);

            BossLine fail3 = {};
            StringCchCopyW(fail3.text, BUF_XLARGE, boss.failureMessage);
            fail3.color = Colors::Red;
            fail3.delayMs = 2000;
            g_BossLines.push_back(fail3);

            BossLine cons = {};
            StringCchPrintfW(cons.text, BUF_XLARGE, L"Reputation halved. Cash seized. Skills degraded. Heat reset.");
            cons.color = Colors::Orange;
            cons.delayMs = 2500;
            g_BossLines.push_back(cons);

            break;
        }

        BossLine gap = {};
        StringCchPrintfW(gap.text, BUF_XLARGE, L" ");
        gap.delayMs = 800;
        g_BossLines.push_back(gap);
    }

    if (true == g_BossSucceeded)
    {
        BossLine win1 = {};
        StringCchPrintfW(win1.text, BUF_XLARGE, L" ");
        win1.delayMs = 1000;
        g_BossLines.push_back(win1);

        BossLine win2 = {};
        StringCchPrintfW(win2.text, BUF_XLARGE, L"=== OPERATION SUCCESSFUL ===");
        win2.color = Colors::Green;
        win2.delayMs = 2000;
        g_BossLines.push_back(win2);

        BossLine win3 = {};
        StringCchCopyW(win3.text, BUF_XLARGE, boss.successMessage);
        win3.color = Colors::Green;
        win3.delayMs = 2000;
        g_BossLines.push_back(win3);

        auto rewardBuf = std::make_unique<wchar_t[]>(BUF_LARGE);
        FormatCurrency(boss.successCashReward, rewardBuf.get(), BUF_LARGE);
        BossLine win4 = {};
        StringCchPrintfW(win4.text, BUF_XLARGE, L"Reward: %s  |  +%d reputation", rewardBuf.get(), boss.successRepReward);
        win4.color = Colors::Yellow;
        win4.delayMs = 2000;
        g_BossLines.push_back(win4);
    }
}

// ============================================================================
// Boss Window Procedure
// ============================================================================

constexpr int kBossAutoCloseDelayMs = 3000;
constexpr int kBossLineHeight = 18;
constexpr int kBossTextMarginY = 5;
constexpr int kBossTextMarginX = 8;
constexpr int kBossVisiblePaddingY = 10;
constexpr int kBossFontSize = -13;
constexpr int kBossSkillDegradeDivisor = 2;

LRESULT CALLBACK BossWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
        case WM_TIMER:
        {
            if (TIMER_BOSS + 1 == wParam)
            {
                DestroyWindow(hWnd);
                return 0;
            }
            if (TIMER_BOSS != wParam)
            {
                return 0;
            }

            g_BossDelayAccumulator += BOSS_TICK_MS;
            if (g_BossCurrentLine >= static_cast<int>(g_BossLines.size()))
            {
                return 0;
            }
            if (g_BossDelayAccumulator < g_BossLines[g_BossCurrentLine].delayMs)
            {
                return 0;
            }

            g_BossDelayAccumulator = 0;
            g_BossCurrentLine++;
            InvalidateRect(hWnd, NULL, FALSE);

            if (g_BossCurrentLine < static_cast<int>(g_BossLines.size()))
            {
                return 0;
            }

            // All lines shown — apply consequences
            const BossHackDef& boss = GetBossHackDef(g_BossIdx);
            s_pGameState->heat = 0;

            if (true == g_BossSucceeded)
            {
                s_pGameState->tierBossCompleted[g_BossIdx] = true;
                s_pGameState->cash += boss.successCashReward;
                s_pGameState->reputation += boss.successRepReward;
                AddEventLog(s_pGameState, boss.successMessage, Colors::Green);
                AddToast(s_pGameState, L"BOSS HACK SUCCEEDED!", Colors::Green);

                // T4 boss completion unlocks military cyber enlistment option
                if (0 == g_BossIdx && false == s_pGameState->cyberVeteran)
                {
                    s_pGameState->militaryEnlistAvailable = true;
                    AddEventLog(s_pGameState, L"A recruiter noticed your skills. Military Cyber Operations enlistment available.", Colors::Olive);
                    AddToast(s_pGameState, L"Military Cyber route unlocked! Check Military tab.", Colors::Olive);
                }
            }
            else
            {
                s_pGameState->reputation /= kBossSkillDegradeDivisor;
                s_pGameState->cash = 0;
                for (int i = 0; i < NUM_SKILLS; i++)
                {
                    // Degrade, but never below the player's earned checkpoint
                    // (25/50/75/100 milestones) -- training you don't forget.
                    int degraded = s_pGameState->skills[i].level / kBossSkillDegradeDivisor;
                    if (degraded < s_pGameState->skillCheckpoints[i])
                    {
                        degraded = s_pGameState->skillCheckpoints[i];
                    }
                    s_pGameState->skills[i].level = degraded;
                    s_pGameState->skills[i].xp = 0;
                }
                s_pGameState->bossAttemptCooldown[g_BossIdx] = boss.cooldownSeconds;
                AddEventLog(s_pGameState, boss.failureMessage, Colors::Red);
                AddToast(s_pGameState, L"BOSS HACK FAILED!", Colors::Red);
            }

            ZeroMemory(s_pGameState->bossPrepDone[g_BossIdx], sizeof(s_pGameState->bossPrepDone[g_BossIdx]));

            SetTimer(hWnd, TIMER_BOSS + 1, kBossAutoCloseDelayMs, NULL);
            KillTimer(hWnd, TIMER_BOSS);
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

            HBRUSH bgBrush = CreateSolidBrush(Colors::BackgroundPrimary);
            FillRect(hdc, &clientRc, bgBrush);
            DeleteObject(bgBrush);

            if (nullptr == g_BossFont)
            {
                g_BossFont = CreateFontW(kBossFontSize, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                    DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                    CLEARTYPE_QUALITY, FIXED_PITCH | FF_MODERN, L"Consolas");
            }

            HFONT oldFont = static_cast<HFONT>(SelectObject(hdc, g_BossFont));
            SetBkMode(hdc, TRANSPARENT);

            int maxVisibleLines = (h - kBossVisiblePaddingY) / kBossLineHeight;
            int startLine = 0;
            if (g_BossCurrentLine > maxVisibleLines)
            {
                startLine = g_BossCurrentLine - maxVisibleLines;
            }

            int drawY = kBossTextMarginY;
            for (int i = startLine; i < g_BossCurrentLine && i < static_cast<int>(g_BossLines.size()); i++)
            {
                SetTextColor(hdc, g_BossLines[i].color);
                TextOutW(hdc, kBossTextMarginX, drawY, g_BossLines[i].text, lstrlenW(g_BossLines[i].text));
                drawY += kBossLineHeight;
            }

            SelectObject(hdc, oldFont);
            EndPaint(hWnd, &ps);
            return 0;
        }

        case WM_ERASEBKGND:
            return 1;

        case WM_DESTROY:
            KillTimer(hWnd, TIMER_BOSS);
            KillTimer(hWnd, TIMER_BOSS + 1);
            g_BossWnd = NULL;
            if (nullptr != g_BossFont)
            {
                DeleteObject(g_BossFont);
                g_BossFont = NULL;
            }
            return 0;
    }
    return DefWindowProcW(hWnd, msg, wParam, lParam);
}

// ============================================================================
// Boss Window Spawning
// ============================================================================

constexpr int kBossWindowWidth = 600;
constexpr int kBossWindowHeight = 450;

void SpawnBossWindow(GameState* pGameState, HWND hMainWnd, int bossIdx)
{
    if (nullptr != g_BossWnd)
    {
        DestroyWindow(g_BossWnd);
    }

    s_pGameState = pGameState;
    ExecuteBossHack(pGameState, bossIdx);

    const BossHackDef& boss = GetBossHackDef(bossIdx);

    auto title = std::make_unique<wchar_t[]>(BUF_LARGE);
    StringCchPrintfW(title.get(), BUF_LARGE, L"SHADOWNET // %s", boss.operationName);

    int screenW = GetSystemMetrics(SM_CXSCREEN);
    int screenH = GetSystemMetrics(SM_CYSCREEN);

    g_BossWnd = CreateWindowExW(
        WS_EX_TOOLWINDOW,
        L"ShadowNetBossClass",
        title.get(),
        WS_POPUP | WS_CAPTION | WS_SYSMENU,
        (screenW - kBossWindowWidth) / 2,
        (screenH - kBossWindowHeight) / 2,
        kBossWindowWidth, kBossWindowHeight,
        hMainWnd,
        NULL,
        GetModuleHandle(NULL),
        NULL);

    if (nullptr != g_BossWnd)
    {
        ShowWindow(g_BossWnd, SW_SHOW);
        SetTimer(g_BossWnd, TIMER_BOSS, BOSS_TICK_MS, NULL);
    }
}

// ============================================================================
// Cleanup
// ============================================================================

void CleanupBossWindow()
{
    if (nullptr != g_BossWnd)
    {
        DestroyWindow(g_BossWnd);
        g_BossWnd = NULL;
    }
}
