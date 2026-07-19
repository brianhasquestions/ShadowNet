#pragma once

#include <windows.h>
#include <vector>
#include "../core/game.h"

// ============================================================================
// IRC Window Constants
// ============================================================================

constexpr UINT_PTR TIMER_IRC = 10;
constexpr int IRC_TICK_MS = 50;

// ============================================================================
// IRC Message Structure
// ============================================================================

struct IRCMessage
{
    wchar_t sender[BUF_SMALL];
    wchar_t text[BUF_XLARGE];
    COLORREF color;
    int delayMs;
};

// ============================================================================
// IRC Window State (module-internal, exposed for main.cpp access)
// ============================================================================

extern HWND g_IRCWnd;

// ============================================================================
// IRC Window Functions
// ============================================================================

void SpawnIRCWindow(GameState* pGameState, HWND hMainWnd, int typeIndex, bool isSuccess, const wchar_t* personnelHandle);
void SpawnJobUnlockIRCWindow(GameState* pGameState, HWND hMainWnd, int jobDefId);
LRESULT CALLBACK IRCWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
void CleanupIRCWindow();
