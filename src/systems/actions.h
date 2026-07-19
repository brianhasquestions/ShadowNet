#pragma once

#include "../core/game.h"

// ============================================================================
// Game Action Functions (Personnel, Market, Prestige)
// ============================================================================

// Personnel hiring with IRC window integration
// Requires function pointers for spawning IRC/melt windows since those are in the windows module
using SpawnIRCWindowFunc = void(*)(GameState*, HWND, int, bool, const wchar_t*);
using SpawnScreenMeltFunc = void(*)(const wchar_t*);

void HirePersonnel(GameState* pGameState, int typeIndex, SpawnIRCWindowFunc spawnIRC, HWND hMainWnd, SpawnScreenMeltFunc spawnMelt);
void BuyMarketItem(GameState* pGameState, int itemIndex);
void SellMarketItem(GameState* pGameState, int itemIndex);
void ActivatePrestige(GameState* pGameState);
