#include "skills.h"

// ============================================================================
// Skill Bonus Constants
// ============================================================================

constexpr int kScarcityLevel100 = 100;
constexpr int kScarcityLevel75 = 75;
constexpr int kScarcityLevel50 = 50;
constexpr int kScarcityLevel25 = 25;
constexpr float kScarcityBonus100 = 0.50f;
constexpr float kScarcityBonus75 = 0.30f;
constexpr float kScarcityBonus50 = 0.15f;
constexpr float kScarcityBonus25 = 0.05f;

constexpr float kSpeedBonusPerLevel = 0.005f;
constexpr float kSpeedBonusPerPrestige = 0.05f;
constexpr float kAISpeedBonusPerLevel = 0.003f;
constexpr float kAISpeedBonusCap = 0.30f;
constexpr float kTotalSpeedBonusCap = 0.70f;

constexpr float kPayoutBonusPerLevel = 0.0025f;
constexpr float kPayoutBonusPerPrestige = 0.03f;

// ============================================================================
// Multitasking Constants
// ============================================================================

constexpr float kMultitask2TimeMult = 1.05f;
constexpr float kMultitask2XPMult = 0.95f;
constexpr float kMultitask3TimeMult = 1.15f;
constexpr float kMultitask3XPMult = 0.85f;
constexpr float kMultitask4TimeMult = 1.30f;
constexpr float kMultitask4XPMult = 0.70f;
constexpr float kMultitask5PlusTimeMult = 1.50f;
constexpr float kMultitask5PlusXPMult = 0.50f;

// ============================================================================
// Scarcity Bonus
// ============================================================================

static float GetScarcityBonus(int level)
{
    if (level >= kScarcityLevel100)
    {
        return kScarcityBonus100;
    }
    if (level >= kScarcityLevel75)
    {
        return kScarcityBonus75;
    }
    if (level >= kScarcityLevel50)
    {
        return kScarcityBonus50;
    }
    if (level >= kScarcityLevel25)
    {
        return kScarcityBonus25;
    }
    return 0.0f;
}

// ============================================================================
// Skill Speed Bonus
// ============================================================================

float GetSkillSpeedBonus(const GameState* pGameState, int jobDefId)
{
    const JobDef& job = GetJobDef(jobDefId);
    float totalBonus = 0.0f;
    for (int i = 0; i < job.numSkillsTrained; i++)
    {
        int level = pGameState->skills[job.skillsTrained[i].skill].level;
        totalBonus += level * kSpeedBonusPerLevel;
    }
    totalBonus += pGameState->prestigeLevel * kSpeedBonusPerPrestige;

    int aiLevel = pGameState->skills[SKILL_AI_ML].level;
    float aiBonus = aiLevel * kAISpeedBonusPerLevel;
    if (aiBonus > kAISpeedBonusCap)
    {
        aiBonus = kAISpeedBonusCap;
    }
    totalBonus += aiBonus;

    if (totalBonus > kTotalSpeedBonusCap)
    {
        totalBonus = kTotalSpeedBonusCap;
    }
    return totalBonus;
}

// ============================================================================
// Skill Payout Bonus
// ============================================================================

float GetSkillPayoutBonus(const GameState* pGameState, int jobDefId)
{
    const JobDef& job = GetJobDef(jobDefId);
    float totalBonus = 0.0f;
    for (int i = 0; i < job.numSkillsTrained; i++)
    {
        int level = pGameState->skills[job.skillsTrained[i].skill].level;
        totalBonus += level * kPayoutBonusPerLevel;
        totalBonus += GetScarcityBonus(level);
    }
    totalBonus += pGameState->prestigeLevel * kPayoutBonusPerPrestige;
    return totalBonus;
}

// ============================================================================
// Multitasking Penalties
// ============================================================================

int GetNonExemptJobCount(const GameState* pGameState)
{
    int count = 0;
    for (int i = 0; i < MAX_ACTIVE_JOBS; i++)
    {
        if (false == pGameState->activeJobs[i].active)
        {
            continue;
        }
        int jid = pGameState->activeJobs[i].jobDefId;
        if (true == IsPassiveJob(jid))
        {
            continue;
        }
        count++;
    }
    return count;
}

MultitaskPenalty GetMultitaskPenalty(const GameState* pGameState)
{
    int count = GetNonExemptJobCount(pGameState);
    MultitaskPenalty p = { 1.0f, 1.0f };
    if (count <= 1)
    {
        return p;
    }
    if (2 == count)
    {
        p.timeMult = kMultitask2TimeMult;
        p.xpMult = kMultitask2XPMult;
    }
    else if (3 == count)
    {
        p.timeMult = kMultitask3TimeMult;
        p.xpMult = kMultitask3XPMult;
    }
    else if (4 == count)
    {
        p.timeMult = kMultitask4TimeMult;
        p.xpMult = kMultitask4XPMult;
    }
    else
    {
        p.timeMult = kMultitask5PlusTimeMult;
        p.xpMult = kMultitask5PlusXPMult;
    }
    return p;
}
