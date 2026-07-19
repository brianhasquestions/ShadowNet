#pragma once

#include "../core/game.h"

// ============================================================================
// Skill Bonus Calculations
// ============================================================================

float GetSkillSpeedBonus(const GameState* pGameState, int jobDefId);
float GetSkillPayoutBonus(const GameState* pGameState, int jobDefId);

// ============================================================================
// Multitasking Penalties
// ============================================================================

int GetNonExemptJobCount(const GameState* pGameState);
MultitaskPenalty GetMultitaskPenalty(const GameState* pGameState);
