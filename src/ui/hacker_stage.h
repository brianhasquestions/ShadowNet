#pragma once
#include "ui.h"

// ============================================================================
// Hacker Stage
//
// A pink / blue cyberpunk pixel-art strip that sits above the gameplay panel.
// A hooded hacker bobs their head and types at a desk, and the scene reacts to
// what happens in the game: jobs running, paydays, heat, raids, jail. While
// enlisted the same hacker sits in a windowless military cubicle in uniform.
//
// The stage never writes to GameState. It observes it once per render tick,
// diffs a few counters (jobs completed, event log head, jail flag, ...) and
// drives its own animation state machine from that.
// ============================================================================

// Height of the compact stage drawn at the top of the minimized overlay.
// The full strip height is STAGE_HEIGHT in ui.h with the other layout constants.
constexpr int STAGE_COMPACT_HEIGHT = 68;

// Advance animation timers and detect game events. Call once per render tick.
void HackerStageUpdate(const GameState* gs, int dtMs);

// Draw the full strip with the readout into rc (x = SIDEBAR_WIDTH, y = 0, h = STAGE_HEIGHT).
void DrawHackerStage(const DrawCtx& ctx, const GameState* gs, PanelRect rc);

// Draw the half-scale scene with a short caption into rc (the top of the overlay).
void DrawHackerStageCompact(const DrawCtx& ctx, const GameState* gs, PanelRect rc);
