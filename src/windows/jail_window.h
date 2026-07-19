#pragma once

#include <windows.h>
#include <cstdint>

// ============================================================================
// Jail Window Constants
// ============================================================================

constexpr UINT_PTR TIMER_JAIL = 13;
constexpr int JAIL_TICK_MS = 16;               // ~60fps
constexpr int JAIL_BAR_COUNT = 12;
constexpr int JAIL_BAR_WIDTH = 20;
constexpr int JAIL_DESCEND_DURATION_TICKS = 188; // ~3 seconds at 16ms
constexpr int JAIL_POLICE_FLASH_INTERVAL = 30;   // ticks between red/blue flash

// ============================================================================
// Jail Window State
// ============================================================================

extern HWND g_JailWnd;

// ============================================================================
// Jail Window Functions
// ============================================================================

void SpawnJailWindow(int64_t jailEndTimestamp);
LRESULT CALLBACK JailWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
void CleanupJailWindow();
