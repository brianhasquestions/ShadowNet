#pragma once
#include "../core/game.h"

// ============================================================================
// Hacker Usernames (500+ for IRC chat dialogues)
// ============================================================================

extern const wchar_t* g_HackerUsernames[];
extern const int NUM_HACKER_USERNAMES;

// ============================================================================
// Intel Market Item Definitions
// ============================================================================

extern const IntelItemDef g_IntelItems[];

// ============================================================================
// IRC Dialogue Template Pools (for dynamic chat generation)
// ============================================================================

// Success greeting lines - the recruit introduces themselves
extern const wchar_t* g_IRCGreetingsSuccess[];
extern const int NUM_IRC_GREETINGS_SUCCESS;

// Scam greeting lines - the scammer sets the trap
extern const wchar_t* g_IRCGreetingsScam[];
extern const int NUM_IRC_GREETINGS_SCAM;

// Specialty banter - lines about the recruit's skill area
struct SpecialtyLine
{
    PersonnelType type;     // Which personnel type this applies to
    const wchar_t* text;
};
extern const SpecialtyLine g_IRCSpecialtyLines[];
extern const int NUM_IRC_SPECIALTY_LINES;

// Generic banter lines anyone might say
extern const wchar_t* g_IRCBanterLines[];
extern const int NUM_IRC_BANTER_LINES;

// Success outcome lines
extern const wchar_t* g_IRCOutcomeSuccess[];
extern const int NUM_IRC_OUTCOME_SUCCESS;

// Scam outcome lines
extern const wchar_t* g_IRCOutcomeScam[];
extern const int NUM_IRC_OUTCOME_SCAM;

// Job unlock dialogue lines - lines about discovering new jobs
extern const wchar_t* g_IRCJobUnlockIntros[];
extern const int NUM_IRC_JOB_UNLOCK_INTROS;

extern const wchar_t* g_IRCJobUnlockClosers[];
extern const int NUM_IRC_JOB_UNLOCK_CLOSERS;
