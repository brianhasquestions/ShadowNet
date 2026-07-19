#pragma once

#include <windows.h>

// ============================================================================
// Screen Melt Constants
// ============================================================================

constexpr UINT_PTR TIMER_MELT = 12;
constexpr int MELT_TICK_MS = 16;              // ~60fps
constexpr int MELT_DURATION_TICKS = 3750;     // ~60 seconds at 16ms
constexpr int MELT_MAX_MATRIX_COLS = 120;
constexpr int MELT_MATRIX_CHAR_LEN = 32;
constexpr int MELT_COL_WIDTH_PX = 14;

// ============================================================================
// Matrix Rain Column
// ============================================================================

struct MatrixColumn
{
    int x;
    int y;
    int speed;
    int length;
    wchar_t chars[MELT_MATRIX_CHAR_LEN];
};

// ============================================================================
// Screen Melt State
// ============================================================================

extern HWND g_MeltWnd;

// ============================================================================
// Screen Melt Functions
// ============================================================================

void SpawnScreenMelt(const wchar_t* attackerName);
LRESULT CALLBACK MeltWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
void CleanupMeltWindow();
