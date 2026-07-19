#pragma once

#include <windows.h>
#include <vector>
#include "../core/game.h"

// ============================================================================
// Boss Window Constants
// ============================================================================

constexpr UINT_PTR TIMER_BOSS = 11;
constexpr int BOSS_TICK_MS = 50;

// ============================================================================
// Boss Line Structure
// ============================================================================

struct BossLine
{
    wchar_t text[BUF_XLARGE];
    COLORREF color;
    int delayMs;
    bool isSkillCheck;
    bool isResult;
    bool passed;
};

// ============================================================================
// Boss Window State
// ============================================================================

extern HWND g_BossWnd;

// ============================================================================
// Boss Window Functions
// ============================================================================

void SpawnBossWindow(GameState* pGameState, HWND hMainWnd, int bossIdx);
LRESULT CALLBACK BossWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
void CleanupBossWindow();
