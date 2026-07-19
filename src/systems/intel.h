#pragma once

#include "../core/game.h"

// ============================================================================
// Intel Market System
// ============================================================================

float GetIntelTimeReduction(const GameState* pGameState, int jobDefId);
void RefreshIntelMarketStock(GameState* pGameState);
