#include "game.h"
#include "../ui/colors.h"
#include "../data/strings.h"
#include "../systems/skills.h"
#include "../systems/intel.h"
#include "../systems/events.h"
#include "../systems/offline.h"
#include <strsafe.h>
#include <memory>
#include <cmath>
#include <ctime>

// ============================================================================
// Personnel Type Definitions
// ============================================================================

extern const PersonnelTypeDef g_PersonnelTypes[static_cast<int>(PersonnelType::COUNT)] = {
    { L"Script Kiddie",             500,    SKILL_WEB_APP_SECURITY,    0 },
    { L"Forum Lurker",             200,    SKILL_SOCIAL_ENGINEERING,  0 },
    { L"Phisher",                  2000,   SKILL_SOCIAL_ENGINEERING,  100 },
    { L"Malware Coder",            5000,   SKILL_WINDOWS_EXPLOITATION,100 },
    { L"Carder",                   3000,   SKILL_FINANCIAL_SYSTEMS,   100 },
    { L"Network Specialist",       8000,   SKILL_NETWORK_ENGINEERING, 500 },
    { L"Exploit Developer",        20000,  SKILL_REVERSE_ENGINEERING, 500 },
    { L"Data Broker",              15000,  SKILL_DATABASE_SYSTEMS,    500 },
    { L"Money Launderer",          25000,  SKILL_FINANCIAL_SYSTEMS,   500 },
    { L"OPSEC Specialist",         30000,  SKILL_OPSEC,               2000 },
    { L"Zero-Day Researcher",      100000, SKILL_REVERSE_ENGINEERING, 2000 },
    { L"Insider Handler",          50000,  SKILL_SOCIAL_ENGINEERING,  2000 },
    { L"Infrastructure Architect", 75000,  SKILL_UNIX_LINUX,          2000 },
};

// Hacker handle parts for random name generation
extern const wchar_t* g_HandlePrefixes[] = {
    L"zer0", L"dark", L"null", L"ph4nt0m", L"sh4d0w", L"bl4ck",
    L"n3on", L"crypt0", L"d3ad", L"gh0st", L"c0d3", L"h4x",
    L"r00t", L"v0id", L"gl1tch", L"byt3", L"d4ta", L"n0d3",
    L"pr0xy", L"sk3l3t0n", L"sp1d3r", L"v1p3r", L"w0lf", L"r4v3n",
};
extern const wchar_t* g_HandleSuffixes[] = {
    L"_kid", L"_m4n", L"_x", L"_1337", L"_0x", L"_byte",
    L"_net", L"_sys", L"_pwn", L"_hack", L"_r00t", L"_sec",
    L"_ops", L"_dev", L"_mod", L"42", L"99", L"_z",
};
extern const int NUM_PREFIXES = _countof(g_HandlePrefixes);
extern const int NUM_SUFFIXES = _countof(g_HandleSuffixes);

// ============================================================================
// RNG - xorshift32
// ============================================================================

uint32_t GameRandom(GameState* pGameState)
{
    uint32_t x = pGameState->rngState;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    pGameState->rngState = x;
    return x;
}

int GameRandomRange(GameState* pGameState, int minVal, int maxVal)
{
    if (minVal >= maxVal)
    {
        return minVal;
    }
    return minVal + static_cast<int>(GameRandom(pGameState) % static_cast<uint32_t>(maxVal - minVal + 1));
}

// ============================================================================
// Player Title
// ============================================================================

PlayerTitle GetPlayerTitle(int reputation)
{
    if (reputation >= REP_SHADOW_BROKER)
    {
        return PlayerTitle::ShadowBroker;
    }
    if (reputation >= REP_TIER1_UNLOCK)
    {
        return PlayerTitle::APTOperator;
    }
    if (reputation >= REP_TIER2_UNLOCK)
    {
        return PlayerTitle::ThreatActor;
    }
    if (reputation >= REP_TIER3_UNLOCK)
    {
        return PlayerTitle::BlackHat;
    }
    return PlayerTitle::ScriptKiddie;
}

// ============================================================================
// Slot Calculations
// ============================================================================

int GetMaxJobSlots(const GameState* pGameState)
{
    int base = 1;
    if (pGameState->reputation >= REP_TIER3_UNLOCK)
    {
        base = 2;
    }
    if (pGameState->reputation >= REP_TIER2_UNLOCK)
    {
        base = 3;
    }
    if (pGameState->reputation >= REP_TIER1_UNLOCK)
    {
        base = 5;
    }
    if (pGameState->reputation >= REP_SHADOW_BROKER)
    {
        base = 6;
    }

    constexpr int kSlotsPerServer = 2;
    base += pGameState->inventory.dedicatedServers * kSlotsPerServer;
    base += pGameState->inventory.bulletproofVPS;

    if (base > MAX_ACTIVE_JOBS)
    {
        base = MAX_ACTIVE_JOBS;
    }
    return base;
}

int GetMaxPersonnelSlots(const GameState* pGameState)
{
    int base = 1;
    constexpr int kTier3PersonnelSlots = 3;
    constexpr int kTier2PersonnelSlots = 6;
    constexpr int kTier1PersonnelSlots = 10;
    if (pGameState->reputation >= REP_TIER3_UNLOCK)
    {
        base = kTier3PersonnelSlots;
    }
    if (pGameState->reputation >= REP_TIER2_UNLOCK)
    {
        base = kTier2PersonnelSlots;
    }
    if (pGameState->reputation >= REP_TIER1_UNLOCK)
    {
        base = kTier1PersonnelSlots;
    }
    if (pGameState->prestigeLevel > 0 && pGameState->reputation >= REP_SHADOW_BROKER)
    {
        base = MAX_PERSONNEL;
    }

    base += pGameState->purchasedPersonnelSlots;
    if (base > MAX_PERSONNEL)
    {
        base = MAX_PERSONNEL;
    }
    return base;
}

int GetActiveJobCount(const GameState* pGameState)
{
    int count = 0;
    for (int i = 0; i < MAX_ACTIVE_JOBS; i++)
    {
        if (true == pGameState->activeJobs[i].active)
        {
            count++;
        }
    }
    return count;
}

// ============================================================================
// Prestige & Personnel Slot Cost
// ============================================================================

bool ArePrestigeRequirementsMet(const GameState* pGameState)
{
    if (pGameState->reputation < PRESTIGE_REP_REQUIRED)
    {
        return false;
    }
    if (pGameState->cash < PRESTIGE_CASH_REQUIRED)
    {
        return false;
    }
    int tier1Completed = 0;
    int jobCount = GetJobCount();
    for (int i = 0; i < jobCount; i++)
    {
        const JobDef& job = GetJobDef(i);
        if (Tier::One == job.tier && false == job.isLegitimate && pGameState->jobCompletionCounts[i] > 0)
        {
            tier1Completed++;
        }
    }
    return tier1Completed >= PRESTIGE_TIER1_JOBS_REQUIRED;
}

constexpr int64_t EXTRA_SLOT_BASE_COST = 5000;

int64_t GetNextPersonnelSlotCost(const GameState* pGameState)
{
    return EXTRA_SLOT_BASE_COST * (1LL << pGameState->purchasedPersonnelSlots);
}

int GetBossIndexForTier(Tier tier)
{
    switch (tier)
    {
        case Tier::Three:
            return 0;
        case Tier::Two:
            return 1;
        case Tier::One:
            return 2;
        default:
            return -1;
    }
}

int GetBossRequiredRep(int bossIdx)
{
    switch (bossIdx)
    {
        case 0: return REP_TIER3_UNLOCK;
        case 1: return REP_TIER2_UNLOCK;
        case 2: return REP_TIER1_UNLOCK;
        case 3: return REP_SHADOW_BROKER;
        default: return 0;
    }
}

bool CanAttemptBoss(const GameState* pGameState, int bossIdx)
{
    if (bossIdx < 0 || bossIdx >= NUM_BOSS_HACKS)
    {
        return false;
    }
    if (true == pGameState->tierBossCompleted[bossIdx])
    {
        return false;
    }
    if (pGameState->bossAttemptCooldown[bossIdx] > 0)
    {
        return false;
    }
    return true;
}

// ============================================================================
// Job Unlocking Logic
// ============================================================================

static bool IsJobTierUnlocked(const GameState* pGameState, Tier tier)
{
    switch (tier)
    {
        case Tier::Four:
            return true;
        case Tier::Three:
            return pGameState->reputation >= REP_TIER3_UNLOCK && true == pGameState->tierBossCompleted[0];
        case Tier::Two:
            return pGameState->reputation >= REP_TIER2_UNLOCK && true == pGameState->tierBossCompleted[1];
        case Tier::One:
            return pGameState->reputation >= REP_TIER1_UNLOCK && true == pGameState->tierBossCompleted[2];
    }
    return false;
}

static bool MeetsSkillRequirements(const GameState* pGameState, const JobDef& job)
{
    for (int i = 0; i < job.numSkillReqs; i++)
    {
        if (pGameState->skills[job.skillReqs[i].skill].level < job.skillReqs[i].minLevel)
        {
            return false;
        }
    }
    return true;
}

bool CanStartJob(const GameState* pGameState, int jobDefId)
{
    const JobDef& job = GetJobDef(jobDefId);

    if (JOB_ID_FORUM_TIP_FOLLOWUP == jobDefId && false == pGameState->pendingForumTip)
    {
        return false;
    }
    if (true == IsGameHackJob(jobDefId) && pGameState->gameHackBanTimer > 0)
    {
        return false;
    }
    if (true == IsContractorJob(jobDefId) && false == pGameState->cyberVeteran)
    {
        return false;
    }
    if (true == IsMilitaryActive(pGameState))
    {
        return false;
    }
    if (true == job.isLegitimate && false == pGameState->inPrestigeMode)
    {
        return false;
    }
    if (false == job.isLegitimate && true == pGameState->inPrestigeMode)
    {
        return false;
    }
    if (false == IsJobTierUnlocked(pGameState, job.tier))
    {
        return false;
    }
    if (false == MeetsSkillRequirements(pGameState, job))
    {
        return false;
    }

    if (DocId::COUNT != job.requiredDoc && false == pGameState->docsRead[static_cast<int>(job.requiredDoc)])
    {
        return false;
    }

    if (job.prerequisiteJobId >= 0 && pGameState->jobCompletionCounts[job.prerequisiteJobId] < job.prerequisiteJobCount)
    {
        return false;
    }

    if (GetActiveJobCount(pGameState) >= GetMaxJobSlots(pGameState))
    {
        return false;
    }

    return true;
}

// ============================================================================
// Idle Personnel Selection
//
// Picks an idle (active, not arrested, unassigned) crew member to work a job.
// Prefers a member whose specialty skill the job trains, so hiring the right
// specialist actually matters; otherwise returns the first idle member.
// Returns -1 when no one is available (the job then runs manually).
// ============================================================================

int FindIdlePersonnelForJob(const GameState* pGameState, int jobDefId)
{
    const JobDef& job = GetJobDef(jobDefId);

    int fallback = -1;
    for (int i = 0; i < MAX_PERSONNEL; i++)
    {
        const PersonnelMember& pm = pGameState->personnel[i];
        if (false == pm.active || true == pm.arrested || pm.assignedJobSlot >= 0)
        {
            continue;
        }
        if (fallback < 0)
        {
            fallback = i;
        }
        SkillId spec = g_PersonnelTypes[static_cast<int>(pm.type)].specialty;
        for (int s = 0; s < job.numSkillsTrained; s++)
        {
            if (job.skillsTrained[s].skill == spec)
            {
                return i; // specialty match wins immediately
            }
        }
    }
    return fallback;
}

// ============================================================================
// Start a Job
// ============================================================================

constexpr float kPersonnelTimeReduction = 0.8f;
constexpr float kEvilNginxTimeReduction = 0.85f;
constexpr int kMinJobTime = 1;

// Purchased-tool speed bonuses, applied to jobs training the matching skill.
constexpr float kHardwareDevKitTimeReduction = 0.90f; // hardware/embedded jobs
constexpr float kZeroDayKitTimeReduction = 0.75f;     // exploit (reverse-engineering) jobs
constexpr float kCustomC2TimeReduction = 0.85f;       // malware (Windows exploitation) jobs

bool StartJob(GameState* pGameState, int jobDefId, int personnelId)
{
    if (false == CanStartJob(pGameState, jobDefId))
    {
        return false;
    }

    int slot = -1;
    for (int i = 0; i < MAX_ACTIVE_JOBS; i++)
    {
        if (false == pGameState->activeJobs[i].active)
        {
            slot = i;
            break;
        }
    }
    if (slot < 0)
    {
        return false;
    }

    const JobDef& job = GetJobDef(jobDefId);

    float speedBonus = GetSkillSpeedBonus(pGameState, jobDefId);
    float intelReduction = GetIntelTimeReduction(pGameState, jobDefId);
    int time = static_cast<int>(job.baseTimeSeconds * (1.0f - speedBonus - intelReduction));
    if (time < kMinJobTime)
    {
        time = kMinJobTime;
    }

    // EvilNginx doc bonus for phishing-related jobs
    if (true == pGameState->docsRead[static_cast<int>(DocId::EvilNginxDocs)])
    {
        for (int i = 0; i < job.numSkillsTrained; i++)
        {
            if (SKILL_SOCIAL_ENGINEERING == job.skillsTrained[i].skill)
            {
                time = static_cast<int>(time * kEvilNginxTimeReduction);
                if (time < kMinJobTime)
                {
                    time = kMinJobTime;
                }
                break;
            }
        }
    }

    // Purchased-tool speed bonuses (apply once per matching skill, best-effort).
    {
        bool hwApplied = false, zdApplied = false, c2Applied = false;
        for (int i = 0; i < job.numSkillsTrained; i++)
        {
            SkillId sid = job.skillsTrained[i].skill;
            if (true == pGameState->inventory.hasHardwareDevKit && !hwApplied && SKILL_EMBEDDED_RTOS == sid)
            {
                time = static_cast<int>(time * kHardwareDevKitTimeReduction);
                hwApplied = true;
            }
            if (true == pGameState->inventory.hasZeroDayKit && !zdApplied && SKILL_REVERSE_ENGINEERING == sid)
            {
                time = static_cast<int>(time * kZeroDayKitTimeReduction);
                zdApplied = true;
            }
            if (true == pGameState->inventory.hasCustomC2 && !c2Applied && SKILL_WINDOWS_EXPLOITATION == sid)
            {
                time = static_cast<int>(time * kCustomC2TimeReduction);
                c2Applied = true;
            }
        }
        if (time < kMinJobTime)
        {
            time = kMinJobTime;
        }
    }

    MultitaskPenalty mp = GetMultitaskPenalty(pGameState);
    time = static_cast<int>(time * mp.timeMult);
    if (time < kMinJobTime)
    {
        time = kMinJobTime;
    }

    if (personnelId >= 0)
    {
        time = static_cast<int>(time * kPersonnelTimeReduction);
        if (time < kMinJobTime)
        {
            time = kMinJobTime;
        }
    }

    pGameState->activeJobs[slot].active = true;
    pGameState->activeJobs[slot].jobDefId = jobDefId;
    pGameState->activeJobs[slot].remainingSeconds = time;
    pGameState->activeJobs[slot].totalSeconds = time;
    pGameState->activeJobs[slot].assignedPersonnelId = personnelId;

    pGameState->jobUnlocked[jobDefId] = true;

    if (personnelId >= 0 && personnelId < MAX_PERSONNEL)
    {
        pGameState->personnel[personnelId].assignedJobSlot = slot;
    }

    return true;
}

// ============================================================================
// Event Log & Toasts
// ============================================================================

constexpr int kToastDurationMs = 4000;

void AddEventLog(GameState* pGameState, const wchar_t* text, COLORREF color)
{
    int idx = pGameState->eventLogHead;
    StringCchCopyW(pGameState->eventLog[idx].text, BUF_XLARGE, text);
    pGameState->eventLog[idx].color = color;
    pGameState->eventLog[idx].timestamp = pGameState->totalTicksPlayed;

    pGameState->eventLogHead = (pGameState->eventLogHead + 1) % MAX_EVENT_LOG;
    if (pGameState->eventLogCount < MAX_EVENT_LOG)
    {
        pGameState->eventLogCount++;
    }
}

void AddToast(GameState* pGameState, const wchar_t* text, COLORREF color)
{
    for (int i = 0; i < MAX_TOASTS; i++)
    {
        if (false == pGameState->toasts[i].active)
        {
            StringCchCopyW(pGameState->toasts[i].text, BUF_XLARGE, text);
            pGameState->toasts[i].color = color;
            pGameState->toasts[i].active = true;
            pGameState->toasts[i].remainingMs = kToastDurationMs;
            return;
        }
    }
    StringCchCopyW(pGameState->toasts[0].text, BUF_XLARGE, text);
    pGameState->toasts[0].color = color;
    pGameState->toasts[0].active = true;
    pGameState->toasts[0].remainingMs = kToastDurationMs;
}

// ============================================================================
// Job Completion
// ============================================================================

constexpr int kWebDefacementBustChance = 35;
constexpr int kMinBustChance = 5;
constexpr int kBustChanceOPSECDivisor = 2;
constexpr int kBustHeatGain = 40;
constexpr int kGameHackBanChance = 20;
constexpr int kMinBanChance = 3;
constexpr int kBanChanceREDivisor = 3;
constexpr int kGameHackMinBan = 300;
constexpr int kGameHackMaxBan = 900;
constexpr int kContractorFailBase = 15;
constexpr int kContractorFailMin = 5;
constexpr int kContractorFailOPSECDiv = 7;
constexpr float kPersonnelPayoutMult = 0.70f;
constexpr int kPersonnelLevelUpChance = 20;
constexpr float kPrestigeXPBonusPerLevel = 0.05f;
constexpr float kAIXPPenaltyPerLevel = 0.004f;
constexpr float kMinAIPenalty = 0.60f;
constexpr float kSpecDepthPenaltyFactor = 0.002f;
constexpr int kSpecDepthThreshold = 25;
constexpr int kSkillMilestone25 = 25;
constexpr int kSkillMilestone50 = 50;
constexpr int kSkillMilestone75 = 75;
constexpr int kSkillMilestone100 = 100;
constexpr int kMaxSkillLevel = 100;
constexpr float kHeatCompoundPerJob = 0.15f;
constexpr float kMaxHeatCompound = 3.0f;
constexpr int kHeatCompoundWarning = 3;
constexpr int kHeatCompoundDanger = 5;
constexpr int kHeatCompoundCritical = 8;
constexpr float kOPSECHeatReductionPerLevel = 0.005f;
constexpr float kMentorshipXPMult = 1.25f;
constexpr int kMentorshipDuration = 600;
constexpr int kForumTipDefaultPayout = 50;
constexpr int kGoodForumTipThreshold = 150;
constexpr int kForumTipRepGain = 2;

static void CompleteJob(GameState* pGameState, int slot)
{
    ActiveJob& aj = pGameState->activeJobs[slot];
    const JobDef& job = GetJobDef(aj.jobDefId);

    float payoutBonus = GetSkillPayoutBonus(pGameState, aj.jobDefId);
    float eventMultiplier = pGameState->payoutMultiplier;
    if (eventMultiplier <= 0.0f)
    {
        eventMultiplier = 1.0f;
    }

    // Assigned crew take a cut of the payout, unless the job matches their
    // specialty (a specialist works efficiently enough to keep full value).
    float personnelMult = 1.0f;
    if (aj.assignedPersonnelId >= 0 && aj.assignedPersonnelId < MAX_PERSONNEL)
    {
        personnelMult = kPersonnelPayoutMult;
        SkillId spec = g_PersonnelTypes[static_cast<int>(pGameState->personnel[aj.assignedPersonnelId].type)].specialty;
        for (int i = 0; i < job.numSkillsTrained; i++)
        {
            if (job.skillsTrained[i].skill == spec)
            {
                personnelMult = 1.0f;
                break;
            }
        }
    }

    int64_t payout = static_cast<int64_t>(job.basePayoutUsd * (1.0f + payoutBonus) * eventMultiplier * personnelMult);

    if (1.0f != pGameState->cryptoPayoutMultiplier && pGameState->cryptoPayoutTimer > 0)
    {
        for (int i = 0; i < job.numSkillsTrained; i++)
        {
            if (SKILL_CRYPTOGRAPHY == job.skillsTrained[i].skill || SKILL_FINANCIAL_SYSTEMS == job.skillsTrained[i].skill)
            {
                payout = static_cast<int64_t>(payout * pGameState->cryptoPayoutMultiplier);
                break;
            }
        }
    }

    // Website Defacement: high bust chance
    if (JOB_ID_WEBSITE_DEFACEMENT == aj.jobDefId)
    {
        int bustChance = kWebDefacementBustChance;
        int opsecLevel = pGameState->skills[SKILL_OPSEC].level;
        bustChance -= opsecLevel / kBustChanceOPSECDivisor;
        if (bustChance < kMinBustChance)
        {
            bustChance = kMinBustChance;
        }

        if (GameRandomRange(pGameState, 0, 100) < bustChance)
        {
            int64_t lostCash = pGameState->cash;
            pGameState->cash = 0;
            pGameState->heat += kBustHeatGain;
            if (pGameState->heat > HEAT_MAX)
            {
                pGameState->heat = HEAT_MAX;
            }

            // Reset only this job's own progress, not every unrelated T4 grind.
            pGameState->jobCompletionCounts[JOB_ID_WEBSITE_DEFACEMENT] = 0;

            auto bustMsg = std::make_unique<wchar_t[]>(BUF_XLARGE);
            StringCchPrintfW(bustMsg.get(), BUF_XLARGE, L"BUSTED! FBI traced your defacement. Lost $%lld. Defacement progress reset.", lostCash);
            AddEventLog(pGameState, bustMsg.get(), Colors::Red);
            AddToast(pGameState, L"BUSTED BY THE FEDS!", Colors::Red);

            aj.active = false;
            aj.assignedPersonnelId = -1;
            return;
        }
    }

    // Game hack jobs: chance of getting banned
    if (true == IsGameHackJob(aj.jobDefId))
    {
        int banChance = kGameHackBanChance;
        int reLevel = pGameState->skills[SKILL_REVERSE_ENGINEERING].level;
        banChance -= reLevel / kBanChanceREDivisor;
        if (banChance < kMinBanChance)
        {
            banChance = kMinBanChance;
        }

        if (GameRandomRange(pGameState, 0, 100) < banChance)
        {
            pGameState->gameHackBanTimer = GameRandomRange(pGameState, kGameHackMinBan, kGameHackMaxBan);
            AddEventLog(pGameState, L"HWID BANNED! Your game accounts got flagged. Game hacking on cooldown.", Colors::Orange);
            AddToast(pGameState, L"Game hack ban! Cooldown active.", Colors::Orange);
        }
    }

    // Contractor jobs: catastrophic failure chance -> jail, lose all money + rep, keep skills
    if (true == IsContractorJob(aj.jobDefId))
    {
        int failChance = kContractorFailBase - pGameState->skills[SKILL_OPSEC].level / kContractorFailOPSECDiv;
        if (failChance < kContractorFailMin)
        {
            failChance = kContractorFailMin;
        }

        if (GameRandomRange(pGameState, 0, 100) < failChance)
        {
            // Catastrophic failure: lose all money and reputation, go to jail
            int64_t lostCash = pGameState->cash;
            int lostRep = pGameState->reputation;
            pGameState->cash = 0;
            pGameState->reputation = 0;
            pGameState->jailed = true;
            pGameState->jailEndTimestamp = static_cast<int64_t>(time(NULL)) + CONTRACTOR_JAIL_DURATION;
            pGameState->triggerJail = true;

            // Cancel all active jobs
            for (int j = 0; j < MAX_ACTIVE_JOBS; j++)
            {
                if (true == pGameState->activeJobs[j].active)
                {
                    if (pGameState->activeJobs[j].assignedPersonnelId >= 0 &&
                        pGameState->activeJobs[j].assignedPersonnelId < MAX_PERSONNEL)
                    {
                        pGameState->personnel[pGameState->activeJobs[j].assignedPersonnelId].assignedJobSlot = -1;
                    }
                    pGameState->activeJobs[j].active = false;
                    pGameState->activeJobs[j].assignedPersonnelId = -1;
                }
            }

            auto failMsg = std::make_unique<wchar_t[]>(BUF_XLARGE);
            StringCchPrintfW(failMsg.get(), BUF_XLARGE, L"OPERATION COMPROMISED! Lost $%lld and %d reputation. Congressional investigation launched.", lostCash, lostRep);
            AddEventLog(pGameState, failMsg.get(), Colors::Red);
            AddEventLog(pGameState, L"Skills and experience preserved. You'll need them when you get out.", Colors::Comment);
            AddToast(pGameState, L"CONTRACTOR BURN! Jail for 48 hours!", Colors::Red);

            aj.active = false;
            aj.assignedPersonnelId = -1;
            return;
        }
    }

    pGameState->cash += payout;
    pGameState->totalEarned += payout;
    pGameState->reputation += job.repReward;
    pGameState->totalJobsCompleted++;
    pGameState->jobCompletionCounts[aj.jobDefId]++;

    // Higher-tier data-theft jobs yield a saleable data dump (Market sell side).
    if (false == job.isLegitimate && (Tier::Two == job.tier || Tier::One == job.tier))
    {
        for (int i = 0; i < job.numSkillsTrained; i++)
        {
            if (SKILL_DATABASE_SYSTEMS == job.skillsTrained[i].skill)
            {
                pGameState->inventory.dataDumps++;
                break;
            }
        }
    }

    // Heat compounding
    int actualHeat = job.heatGenerated;
    if (actualHeat > 0)
    {
        bool hasProtection = pGameState->inventory.hasVPN || pGameState->inventory.burnerPhones > 0 ||
                             pGameState->inventory.hasProxyChain || pGameState->inventory.hasTorHiddenService;

        if (false == hasProtection)
        {
            pGameState->consecutiveJobsNoProtection++;
            pGameState->heatCompoundMultiplier = 1.0f + (pGameState->consecutiveJobsNoProtection * kHeatCompoundPerJob);
            if (pGameState->heatCompoundMultiplier > kMaxHeatCompound)
            {
                pGameState->heatCompoundMultiplier = kMaxHeatCompound;
            }
            actualHeat = static_cast<int>(actualHeat * pGameState->heatCompoundMultiplier);

            if (kHeatCompoundWarning == pGameState->consecutiveJobsNoProtection)
            {
                AddEventLog(pGameState, L"Your digital footprint is growing. Feds are starting to notice.", Colors::Orange);
            }
            else if (kHeatCompoundDanger == pGameState->consecutiveJobsNoProtection)
            {
                AddEventLog(pGameState, L"WARNING: Feds are building a case against you! Get a VPN or burner phone!", Colors::Red);
            }
            else if (pGameState->consecutiveJobsNoProtection >= kHeatCompoundCritical)
            {
                AddEventLog(pGameState, L"CRITICAL: You're operating completely exposed. Heat is compounding fast!", Colors::Red);
            }
        }
        else
        {
            pGameState->consecutiveJobsNoProtection = 0;
            pGameState->heatCompoundMultiplier = 1.0f;
        }

        // Flat per-job heat reduction from owned infrastructure (as advertised
        // in the market: VPN 5%, Proxy Chain 10%, Bulletproof VPS 15%).
        float toolHeatMult = 1.0f;
        if (true == pGameState->inventory.hasVPN) { toolHeatMult -= 0.05f; }
        if (true == pGameState->inventory.hasProxyChain) { toolHeatMult -= 0.10f; }
        if (pGameState->inventory.bulletproofVPS > 0) { toolHeatMult -= 0.15f; }
        if (toolHeatMult < 0.30f) { toolHeatMult = 0.30f; }
        actualHeat = static_cast<int>(actualHeat * toolHeatMult);
    }

    pGameState->heat += actualHeat;
    int opsecLevel = pGameState->skills[SKILL_OPSEC].level;
    if (opsecLevel > 0 && actualHeat > 0)
    {
        int reduction = static_cast<int>(actualHeat * opsecLevel * kOPSECHeatReductionPerLevel);
        pGameState->heat -= reduction;
    }
    for (int i = 0; i < MAX_PERSONNEL; i++)
    {
        if (true == pGameState->personnel[i].active && PersonnelType::OPSECSpecialist == pGameState->personnel[i].type)
        {
            pGameState->heat -= 1;
        }
    }
    if (pGameState->heat < 0)
    {
        pGameState->heat = 0;
    }
    if (pGameState->heat > HEAT_MAX)
    {
        pGameState->heat = HEAT_MAX;
    }

    float trainingMult = 1.0f + pGameState->prestigeLevel * kPrestigeXPBonusPerLevel;
    if (pGameState->skillXPMultiplierTimer > 0)
    {
        trainingMult *= pGameState->skillXPMultiplier;
    }

    MultitaskPenalty mp = GetMultitaskPenalty(pGameState);
    trainingMult *= mp.xpMult;

    int aiLevel = pGameState->skills[SKILL_AI_ML].level;
    float aiPenalty = 1.0f - (aiLevel * kAIXPPenaltyPerLevel);
    if (aiPenalty < kMinAIPenalty)
    {
        aiPenalty = kMinAIPenalty;
    }
    trainingMult *= aiPenalty;

    int totalOtherDepth = 0;
    for (int s = 0; s < NUM_SKILLS; s++)
    {
        if (pGameState->skills[s].level > kSpecDepthThreshold)
        {
            totalOtherDepth += pGameState->skills[s].level - kSpecDepthThreshold;
        }
    }

    for (int i = 0; i < job.numSkillsTrained; i++)
    {
        SkillId sid = job.skillsTrained[i].skill;

        int thisDepth = (pGameState->skills[sid].level > kSpecDepthThreshold) ? (pGameState->skills[sid].level - kSpecDepthThreshold) : 0;
        int otherDepth = totalOtherDepth - thisDepth;
        float specMult = 1.0f / (1.0f + otherDepth * kSpecDepthPenaltyFactor);

        int xpGain = static_cast<int>(job.skillsTrained[i].xpGain * trainingMult * specMult);

        if (aiLevel > 0 && xpGain < 1)
        {
            xpGain = 1;
        }
        if (xpGain < 1)
        {
            xpGain = 1;
        }

        pGameState->skills[sid].xp += xpGain;

        while (pGameState->skills[sid].level < kMaxSkillLevel && pGameState->skills[sid].xp >= XpForNextLevel(pGameState->skills[sid].level))
        {
            pGameState->skills[sid].xp -= XpForNextLevel(pGameState->skills[sid].level);
            pGameState->skills[sid].level++;

            if (kSkillMilestone25 == pGameState->skills[sid].level ||
                kSkillMilestone50 == pGameState->skills[sid].level ||
                kSkillMilestone75 == pGameState->skills[sid].level)
            {
                pGameState->skillCheckpoints[sid] = pGameState->skills[sid].level;
            }

            if (kSkillMilestone25 == pGameState->skills[sid].level ||
                kSkillMilestone50 == pGameState->skills[sid].level ||
                kSkillMilestone75 == pGameState->skills[sid].level ||
                kSkillMilestone100 == pGameState->skills[sid].level)
            {
                auto skillMsg = std::make_unique<wchar_t[]>(BUF_XLARGE);
                const wchar_t* tierName = L"Competent";
                if (kSkillMilestone50 == pGameState->skills[sid].level)
                {
                    tierName = L"Specialist";
                }
                if (kSkillMilestone75 == pGameState->skills[sid].level)
                {
                    tierName = L"Expert";
                }
                if (kSkillMilestone100 == pGameState->skills[sid].level)
                {
                    tierName = L"Elite";
                }
                StringCchPrintfW(skillMsg.get(), BUF_XLARGE, L"%s reached %s tier (level %d)!", SkillNames[sid], tierName, pGameState->skills[sid].level);
                AddEventLog(pGameState, skillMsg.get(), Colors::Purple);
                AddToast(pGameState, skillMsg.get(), Colors::Purple);
            }
        }
    }

    if (aj.assignedPersonnelId >= 0 && aj.assignedPersonnelId < MAX_PERSONNEL)
    {
        PersonnelMember& pm = pGameState->personnel[aj.assignedPersonnelId];
        if (true == pm.active && GameRandomRange(pGameState, 0, 100) < kPersonnelLevelUpChance)
        {
            pm.level++;
            auto personnelMsg = std::make_unique<wchar_t[]>(BUF_XLARGE);
            StringCchPrintfW(personnelMsg.get(), BUF_XLARGE, L"Personnel \"%s\" leveled up to %d!", pm.name, pm.level);
            AddEventLog(pGameState, personnelMsg.get(), Colors::Cyan);
        }
        pm.assignedJobSlot = -1;
    }

    auto completionMsg = std::make_unique<wchar_t[]>(BUF_XLARGE);
    StringCchPrintfW(completionMsg.get(), BUF_XLARGE, L"Job completed: %s    +$%lld", job.name, payout);
    AddEventLog(pGameState, completionMsg.get(), Colors::Green);

    if (JOB_ID_IRC_ADMIN == aj.jobDefId)
    {
        pGameState->skillXPMultiplier = kMentorshipXPMult;
        pGameState->skillXPMultiplierTimer = kMentorshipDuration;
        AddEventLog(pGameState, L"IRC mentorship active! +25% skill XP for 10 minutes.", Colors::Cyan);
        AddToast(pGameState, L"IRC mentorship bonus active!", Colors::Cyan);
    }

    // Forum tip follow-up
    if (JOB_ID_FORUM_TIP_FOLLOWUP == aj.jobDefId)
    {
        int64_t tipPayout = pGameState->forumTipPayout;
        if (tipPayout <= 0)
        {
            tipPayout = kForumTipDefaultPayout;
        }
        pGameState->cash += tipPayout;
        pGameState->totalEarned += tipPayout;
        bool wasGoodLead = tipPayout >= kGoodForumTipThreshold;
        if (true == wasGoodLead)
        {
            pGameState->reputation += kForumTipRepGain;
            auto tipMsg = std::make_unique<wchar_t[]>(BUF_XLARGE);
            StringCchPrintfW(tipMsg.get(), BUF_XLARGE, L"Forum tip paid off! +$%lld, +2 rep. Posted results to the forum.", tipPayout);
            AddEventLog(pGameState, tipMsg.get(), Colors::Cyan);
        }
        else
        {
            auto tipMsg = std::make_unique<wchar_t[]>(BUF_XLARGE);
            StringCchPrintfW(tipMsg.get(), BUF_XLARGE, L"Forum tip was a dud. +$%lld. Not worth posting about.", tipPayout);
            AddEventLog(pGameState, tipMsg.get(), Colors::Comment);
        }
        pGameState->pendingForumTip = false;
        pGameState->forumTipPayout = 0;
        pGameState->jobUnlocked[JOB_ID_FORUM_TIP_FOLLOWUP] = false;
    }

    // Track newly unlocked jobs from IRC/BBS
    if (JOB_ID_BBS_MODERATION == aj.jobDefId || JOB_ID_IRC_ADMIN == aj.jobDefId)
    {
        pGameState->jobUnlockFromIRC = true;
    }

    aj.active = false;
    aj.assignedPersonnelId = -1;
}

// ============================================================================
// Military Cyber Route
// ============================================================================

static const wchar_t* g_BusyworkEvents[] = {
    L"Ordered to paint rocks outside the general's office. Again.",
    L"Mowing the base lawn in 110-degree heat.",
    L"Sweeping sand in the desert. The sand is winning.",
    L"Mopping the sidewalk. In the rain.",
    L"Assigned to guard an empty parking lot for 8 hours.",
    L"PowerPoint ranger duty: updating slides nobody will read.",
    L"Inventorying office supplies for the third time this week.",
    L"Standing in formation waiting for a formation about formations.",
    L"Mandatory fun day: trust falls and team building exercises.",
    L"Police call: picking up cigarette butts you didn't smoke.",
    L"Cleaning weapons that haven't been fired.",
    L"Organizing a supply closet that was already organized.",
    L"PT test: run, push-ups, sit-ups. Repeat tomorrow.",
    L"Waiting for a briefing that got pushed back 4 hours.",
    L"Escorting a VIP's luggage across the base.",
    L"Waxing the hallway floors for a visiting colonel.",
    L"Raking leaves in a windstorm. Sisyphus had it easier.",
    L"Compliance training: 6 hours of clicking 'Next' on slides.",
    L"Safety briefing: don't do dumb things this weekend.",
    L"Assigned to count MREs in the warehouse. All 12,000 of them.",
};

static const wchar_t* g_CyberWorkEvents[] = {
    L"Actually writing a detection rule for the SOC.",
    L"Analyzing a malware sample. First real cyber work this week.",
    L"Penetration test on base network. Found 47 default passwords.",
    L"Updating the SIEM. It crashes twice.",
    L"Writing a Python script to automate a report nobody reads.",
    L"Reviewing firewall logs. 99% false positives.",
    L"Patching servers at 0200. Sleep is for civilians.",
    L"Teaching an O-5 how to use encrypted email. For the 4th time.",
};

constexpr int kMilitaryTrainingXPPerTick = 50;

bool IsMilitaryActive(const GameState* pGameState)
{
    return pGameState->militaryPhase == MilitaryPhase::Training ||
           pGameState->militaryPhase == MilitaryPhase::SpecialtyPick ||
           pGameState->militaryPhase == MilitaryPhase::ActiveDuty;
}

void EnlistMilitary(GameState* pGameState)
{
    if (MilitaryPhase::None != pGameState->militaryPhase)
    {
        return;
    }

    pGameState->militaryPhase = MilitaryPhase::Training;
    pGameState->militaryEnlistTimestamp = static_cast<int64_t>(time(NULL));
    pGameState->militaryDayStartTimestamp = pGameState->militaryEnlistTimestamp;
    pGameState->militaryDaysCompleted = 0;
    pGameState->militaryPayTimer = 0;
    pGameState->militaryTotalPay = 0;
    pGameState->militaryBusyworkTimer = 0;
    pGameState->militaryEnlistAvailable = false;
    pGameState->currentTab = Tab::Military;

    // Cancel all active jobs on enlistment
    for (int i = 0; i < MAX_ACTIVE_JOBS; i++)
    {
        if (true == pGameState->activeJobs[i].active)
        {
            if (pGameState->activeJobs[i].assignedPersonnelId >= 0 &&
                pGameState->activeJobs[i].assignedPersonnelId < MAX_PERSONNEL)
            {
                pGameState->personnel[pGameState->activeJobs[i].assignedPersonnelId].assignedJobSlot = -1;
            }
            pGameState->activeJobs[i].active = false;
            pGameState->activeJobs[i].assignedPersonnelId = -1;
        }
    }

    AddEventLog(pGameState, L"You have enlisted in Military Cyber Operations.", Colors::Olive);
    AddEventLog(pGameState, L"Training begins now. All civilian operations suspended.", Colors::Olive);
    AddToast(pGameState, L"Welcome to the military, soldier.", Colors::Olive);
}

void SelectMilitarySpecialty(GameState* pGameState, SkillId specialty)
{
    if (MilitaryPhase::SpecialtyPick != pGameState->militaryPhase)
    {
        return;
    }

    pGameState->militarySpecialty = specialty;
    pGameState->militaryPhase = MilitaryPhase::ActiveDuty;
    pGameState->militaryDayStartTimestamp = static_cast<int64_t>(time(NULL));

    auto msg = std::make_unique<wchar_t[]>(BUF_XLARGE);
    StringCchPrintfW(msg.get(), BUF_XLARGE, L"Specialty selected: %s. Active duty begins.", SkillNames[specialty]);
    AddEventLog(pGameState, msg.get(), Colors::Olive);
    AddToast(pGameState, L"Active duty. 3 days remaining.", Colors::Olive);
}

void CompleteMilitaryService(GameState* pGameState)
{
    pGameState->militaryPhase = MilitaryPhase::Completed;
    pGameState->cyberVeteran = true;

    // Unlock all tiers automatically
    for (int i = 0; i < NUM_BOSS_HACKS; i++)
    {
        pGameState->tierBossCompleted[i] = true;
    }

    // Ensure minimum reputation for T1 access
    if (pGameState->reputation < REP_TIER1_UNLOCK)
    {
        pGameState->reputation = REP_TIER1_UNLOCK;
    }

    // Unlock all non-contractor, non-prestige jobs
    int jobCount = GetJobCount();
    for (int i = 0; i < jobCount; i++)
    {
        const JobDef& job = GetJobDef(i);
        if (true == job.isLegitimate)
        {
            continue;
        }
        if (true == IsContractorJob(i))
        {
            continue;
        }
        pGameState->jobUnlocked[i] = true;
    }

    // Unlock contractor jobs
    for (int i = JOB_ID_CONTRACTOR_FIRST; i <= JOB_ID_CONTRACTOR_LAST; i++)
    {
        pGameState->jobUnlocked[i] = true;
    }

    AddEventLog(pGameState, L"Honorable discharge. DD-214 in hand. Welcome to the private sector.", Colors::Green);
    AddEventLog(pGameState, L"All tiers unlocked. Contractor jobs now available.", Colors::Green);
    AddToast(pGameState, L"CYBER VETERAN: Contractor tier unlocked!", Colors::Green);
}

void ProcessMilitaryTick(GameState* pGameState)
{
    int64_t now = static_cast<int64_t>(time(NULL));
    int64_t dayElapsed = now - pGameState->militaryDayStartTimestamp;

    // Biweekly pay processing (applies to all active phases)
    pGameState->militaryPayTimer++;
    if (pGameState->militaryPayTimer >= MILITARY_PAY_INTERVAL)
    {
        pGameState->militaryPayTimer = 0;
        pGameState->cash += MILITARY_BASE_PAY;
        pGameState->totalEarned += MILITARY_BASE_PAY;
        pGameState->militaryTotalPay += MILITARY_BASE_PAY;

        auto payMsg = std::make_unique<wchar_t[]>(BUF_XLARGE);
        StringCchPrintfW(payMsg.get(), BUF_XLARGE, L"Military pay deposited: +$%lld (Total: $%lld)", MILITARY_BASE_PAY, pGameState->militaryTotalPay);
        AddEventLog(pGameState, payMsg.get(), Colors::Olive);
    }

    if (MilitaryPhase::Training == pGameState->militaryPhase)
    {
        // Progressive skill training toward level 65
        for (int i = 0; i < NUM_SKILLS; i++)
        {
            if (pGameState->skills[i].level < MILITARY_SKILL_CAP_TRAINING)
            {
                pGameState->skills[i].xp += kMilitaryTrainingXPPerTick;
                while (pGameState->skills[i].level < MILITARY_SKILL_CAP_TRAINING &&
                       pGameState->skills[i].xp >= XpForNextLevel(pGameState->skills[i].level))
                {
                    pGameState->skills[i].xp -= XpForNextLevel(pGameState->skills[i].level);
                    pGameState->skills[i].level++;

                    // Update checkpoints
                    if (25 == pGameState->skills[i].level || 50 == pGameState->skills[i].level)
                    {
                        pGameState->skillCheckpoints[i] = pGameState->skills[i].level;
                    }
                }
            }
        }

        // Busywork events during training (less frequent)
        pGameState->militaryBusyworkTimer++;
        if (pGameState->militaryBusyworkTimer >= MILITARY_BUSYWORK_INTERVAL * 2)
        {
            pGameState->militaryBusyworkTimer = 0;
            int idx = GameRandomRange(pGameState, 0, NUM_BUSYWORK_EVENTS - 1);
            AddEventLog(pGameState, g_BusyworkEvents[idx], Colors::Comment);
        }

        // Check if training day is complete
        if (dayElapsed >= MILITARY_DAY_SECONDS)
        {
            // Force all skills to exactly 65 if not there yet
            for (int i = 0; i < NUM_SKILLS; i++)
            {
                if (pGameState->skills[i].level < MILITARY_SKILL_CAP_TRAINING)
                {
                    pGameState->skills[i].level = MILITARY_SKILL_CAP_TRAINING;
                    pGameState->skills[i].xp = 0;
                }
                if (pGameState->skills[i].level >= 50 && pGameState->skillCheckpoints[i] < 50)
                {
                    pGameState->skillCheckpoints[i] = 50;
                }
                if (pGameState->skills[i].level >= 25 && pGameState->skillCheckpoints[i] < 25)
                {
                    pGameState->skillCheckpoints[i] = 25;
                }
            }

            pGameState->militaryDaysCompleted = 1;
            pGameState->militaryPhase = MilitaryPhase::SpecialtyPick;
            AddEventLog(pGameState, L"Basic cyber training complete. All skills at 65%.", Colors::Olive);
            AddEventLog(pGameState, L"Choose your specialty in the Military tab.", Colors::Yellow);
            AddToast(pGameState, L"Training complete! Pick your specialty.", Colors::Yellow);
        }
    }
    else if (MilitaryPhase::ActiveDuty == pGameState->militaryPhase)
    {
        // Specialty training (10% of time)
        SkillId spec = pGameState->militarySpecialty;
        if (pGameState->skills[spec].level < MILITARY_SKILL_CAP_SPECIALTY)
        {
            // Train specialty at a rate to reach 100 over 3 days
            pGameState->skills[spec].xp += kMilitaryTrainingXPPerTick / 3;
            while (pGameState->skills[spec].level < MILITARY_SKILL_CAP_SPECIALTY &&
                   pGameState->skills[spec].xp >= XpForNextLevel(pGameState->skills[spec].level))
            {
                pGameState->skills[spec].xp -= XpForNextLevel(pGameState->skills[spec].level);
                pGameState->skills[spec].level++;

                if (75 == pGameState->skills[spec].level || 100 == pGameState->skills[spec].level)
                {
                    pGameState->skillCheckpoints[spec] = pGameState->skills[spec].level;
                    auto skillMsg = std::make_unique<wchar_t[]>(BUF_XLARGE);
                    StringCchPrintfW(skillMsg.get(), BUF_XLARGE, L"%s reached level %d!", SkillNames[spec], pGameState->skills[spec].level);
                    AddEventLog(pGameState, skillMsg.get(), Colors::Purple);
                }
            }
        }

        // Busywork/cyber events
        pGameState->militaryBusyworkTimer++;
        if (pGameState->militaryBusyworkTimer >= MILITARY_BUSYWORK_INTERVAL)
        {
            pGameState->militaryBusyworkTimer = 0;
            int roll = GameRandomRange(pGameState, 0, 99);
            if (roll < MILITARY_BUSYWORK_CHANCE)
            {
                int idx = GameRandomRange(pGameState, 0, NUM_BUSYWORK_EVENTS - 1);
                AddEventLog(pGameState, g_BusyworkEvents[idx], Colors::Comment);
            }
            else
            {
                int idx = GameRandomRange(pGameState, 0, NUM_CYBER_WORK_EVENTS - 1);
                AddEventLog(pGameState, g_CyberWorkEvents[idx], Colors::Cyan);
            }
        }

        // Check if current active duty day is complete
        if (dayElapsed >= MILITARY_DAY_SECONDS)
        {
            pGameState->militaryDaysCompleted++;
            pGameState->militaryDayStartTimestamp = now;

            if (pGameState->militaryDaysCompleted >= MILITARY_SERVICE_DAYS)
            {
                // Force specialty to 100 if not there
                if (pGameState->skills[spec].level < MILITARY_SKILL_CAP_SPECIALTY)
                {
                    pGameState->skills[spec].level = MILITARY_SKILL_CAP_SPECIALTY;
                    pGameState->skills[spec].xp = 0;
                    pGameState->skillCheckpoints[spec] = 100;
                }
                CompleteMilitaryService(pGameState);
            }
            else
            {
                int remaining = MILITARY_SERVICE_DAYS - pGameState->militaryDaysCompleted;
                auto dayMsg = std::make_unique<wchar_t[]>(BUF_XLARGE);
                StringCchPrintfW(dayMsg.get(), BUF_XLARGE, L"Day %d complete. %d day(s) of service remaining.", pGameState->militaryDaysCompleted, remaining);
                AddEventLog(pGameState, dayMsg.get(), Colors::Olive);
            }
        }
    }
    // SpecialtyPick phase: just wait for player to pick, pay continues
}

// ============================================================================
// Init
// ============================================================================

constexpr int kInitialGatewayJob1 = 3;   // Credit Card Skimming
constexpr int kInitialGatewayJob2 = 4;   // Bootleg Media Distribution
constexpr int kInitialGatewayJob3 = 5;   // BBS Moderation
constexpr int kInitialGatewayJob4 = 12;  // Fake Review Services
constexpr int kMarketFluctuationInterval = 300;
constexpr uint32_t kRNGSeed = 0xDEADBEEF;

void InitGameState(GameState* pGameState)
{
    ZeroMemory(pGameState, sizeof(GameState));

    pGameState->cash = 0;
    pGameState->reputation = 0;
    pGameState->heat = 0;
    pGameState->prestigeLevel = 0;
    pGameState->inPrestigeMode = false;
    pGameState->currentTab = Tab::Dashboard;

    pGameState->maxJobSlots = 1;
    pGameState->maxPersonnelSlots = 1;

    pGameState->payoutMultiplier = 1.0f;
    pGameState->exploitPriceMultiplier = 1.0f;
    pGameState->cryptoPayoutMultiplier = 1.0f;
    pGameState->marketPriceBoost = 1.0f;

    for (int i = 0; i < static_cast<int>(MarketItemType::COUNT); i++)
    {
        pGameState->market.priceMultipliers[i] = 1.0f;
    }
    pGameState->market.fluctuationTimer = kMarketFluctuationInterval;

    pGameState->jobUnlocked[kInitialGatewayJob1] = true;
    pGameState->jobUnlocked[kInitialGatewayJob2] = true;
    pGameState->jobUnlocked[kInitialGatewayJob3] = true;
    pGameState->jobUnlocked[kInitialGatewayJob4] = true;

    for (int i = 0; i < MAX_ACTIVE_JOBS; i++)
    {
        pGameState->activeJobs[i].active = false;
        pGameState->activeJobs[i].assignedPersonnelId = -1;
    }

    for (int i = 0; i < MAX_PERSONNEL; i++)
    {
        pGameState->personnel[i].active = false;
        pGameState->personnel[i].assignedJobSlot = -1;
    }

    pGameState->rngState = static_cast<uint32_t>(time(NULL)) ^ kRNGSeed;
    pGameState->lastSaveTimestamp = static_cast<int64_t>(time(NULL));

    pGameState->skillXPMultiplier = 1.0f;
    pGameState->purchasedPersonnelSlots = 0;
    pGameState->docStudyActive = false;

    pGameState->pendingForumTip = false;
    pGameState->forumTipPayout = 0;
    ZeroMemory(pGameState->docStudyPasses, sizeof(pGameState->docStudyPasses));
    ZeroMemory(pGameState->skillCheckpoints, sizeof(pGameState->skillCheckpoints));
    ZeroMemory(pGameState->intelInventory, sizeof(pGameState->intelInventory));
    for (int i = 0; i < static_cast<int>(IntelItemType::COUNT); i++)
    {
        pGameState->intelMarketStock[i] = 0;
        pGameState->intelPriceMultipliers[i] = 1.0f;
    }
    pGameState->intelMarketRefreshTimer = 1;
    pGameState->intelScrollOffset = 0;
    pGameState->gameHackBanTimer = 0;
    pGameState->consecutiveJobsNoProtection = 0;
    pGameState->heatCompoundMultiplier = 1.0f;
    pGameState->pendingJobUnlockId = -1;
    pGameState->jobUnlockFromIRC = false;
    pGameState->triggerScreenMelt = false;
    pGameState->meltAttackerName[0] = L'\0';
    pGameState->scamHireCount = 0;
    pGameState->jailed = false;
    pGameState->jailEndTimestamp = 0;
    pGameState->triggerJail = false;

    // Military cyber route
    pGameState->militaryPhase = MilitaryPhase::None;
    pGameState->militaryEnlistTimestamp = 0;
    pGameState->militaryDaysCompleted = 0;
    pGameState->militaryDayStartTimestamp = 0;
    pGameState->militarySpecialty = SKILL_WINDOWS_EXPLOITATION;
    pGameState->militaryPayTimer = 0;
    pGameState->militaryTotalPay = 0;
    pGameState->militaryEnlistAvailable = false;
    pGameState->cyberVeteran = false;
    pGameState->militaryBusyworkTimer = 0;
    pGameState->militaryScrollOffset = 0;

    AddEventLog(pGameState, L"Welcome to ShadowNet. Your journey begins.", Colors::Cyan);
    AddEventLog(pGameState, L"Start by running some basic jobs from the Jobs panel.", Colors::Comment);
}

// ============================================================================
// Main Game Tick (called every 1 second)
// ============================================================================

constexpr int kLoyaltyDecayInterval = 600;
constexpr int kMinLoyalty = 20;
constexpr int kForensicEvasionMinLevel = 20;
constexpr int kForensicEvasionExtraDecayInterval = 60;
constexpr float kMinMarketMult = 0.5f;
constexpr float kMaxMarketMult = 2.0f;
constexpr int kMarketFluctuationRange = 30;
// Doc study XP formula: starts at 5%, decreases to ~1.2% at pass 100
constexpr float kDocStudyBaseXPMult = 0.05f;
constexpr float kDocStudyXPDecayPerPass = 0.008f;
constexpr float kDocStudyMinXPMult = 0.012f;
constexpr int kCursorBlinkInterval = 2;

void GameTick(GameState* pGameState)
{
    pGameState->totalTicksPlayed++;

    // Jail check — while jailed, skip all gameplay processing
    if (true == pGameState->jailed)
    {
        int64_t now = static_cast<int64_t>(time(NULL));
        if (now >= pGameState->jailEndTimestamp)
        {
            // Release from jail
            pGameState->jailed = false;
            pGameState->jailEndTimestamp = 0;
            pGameState->heat = 0;
            pGameState->consecutiveJobsNoProtection = 0;
            pGameState->heatCompoundMultiplier = 1.0f;
            AddEventLog(pGameState, L"Released from federal custody. Your record has been expunged... for now.", Colors::Cyan);
            AddToast(pGameState, L"You're free. Don't get caught again.", Colors::Cyan);
        }
        else
        {
            // Still jailed — only process autosave and cursor blink
            pGameState->cursorBlinkTimer++;
            if (pGameState->cursorBlinkTimer >= kCursorBlinkInterval)
            {
                pGameState->cursorBlinkTimer = 0;
                pGameState->cursorVisible = !pGameState->cursorVisible;
            }
            pGameState->autosaveCounter++;
            if (pGameState->autosaveCounter >= AUTOSAVE_INTERVAL)
            {
                pGameState->autosaveCounter = 0;
                SaveGame(pGameState);
            }
            return;
        }
    }

    // Military service processing (blocks normal jobs while active)
    if (true == IsMilitaryActive(pGameState))
    {
        ProcessMilitaryTick(pGameState);

        // Still process cursor blink, autosave, boss cooldowns during military
        pGameState->cursorBlinkTimer++;
        if (pGameState->cursorBlinkTimer >= kCursorBlinkInterval)
        {
            pGameState->cursorBlinkTimer = 0;
            pGameState->cursorVisible = !pGameState->cursorVisible;
        }
        pGameState->autosaveCounter++;
        if (pGameState->autosaveCounter >= AUTOSAVE_INTERVAL)
        {
            pGameState->autosaveCounter = 0;
            SaveGame(pGameState);
        }
        return; // Skip all normal job/event processing during service
    }

    for (int i = 0; i < MAX_ACTIVE_JOBS; i++)
    {
        if (false == pGameState->activeJobs[i].active)
        {
            continue;
        }

        int pid = pGameState->activeJobs[i].assignedPersonnelId;
        if (pid >= 0 && pid < MAX_PERSONNEL && true == pGameState->personnel[pid].arrested)
        {
            continue;
        }

        pGameState->activeJobs[i].remainingSeconds--;

        if (pGameState->activeJobs[i].remainingSeconds <= 0)
        {
            CompleteJob(pGameState, i);
        }
    }

    for (int i = 0; i < MAX_PERSONNEL; i++)
    {
        if (false == pGameState->personnel[i].active)
        {
            continue;
        }

        if (true == pGameState->personnel[i].arrested)
        {
            pGameState->personnel[i].arrestTimer--;
            if (pGameState->personnel[i].arrestTimer <= 0)
            {
                pGameState->personnel[i].arrested = false;
                auto releaseMsg = std::make_unique<wchar_t[]>(BUF_XLARGE);
                StringCchPrintfW(releaseMsg.get(), BUF_XLARGE, L"\"%s\" released from custody.", pGameState->personnel[i].name);
                AddEventLog(pGameState, releaseMsg.get(), Colors::Cyan);
            }
        }

        if (0 == pGameState->totalTicksPlayed % kLoyaltyDecayInterval && pGameState->personnel[i].loyalty > kMinLoyalty)
        {
            pGameState->personnel[i].loyalty--;
        }
    }

    if (0 == pGameState->totalTicksPlayed % HEAT_DECAY_INTERVAL && pGameState->heat > 0)
    {
        pGameState->heat--;
        int feLevel = pGameState->skills[SKILL_FORENSIC_EVASION].level;
        if (feLevel >= kForensicEvasionMinLevel && pGameState->heat > 0 && 0 == pGameState->totalTicksPlayed % kForensicEvasionExtraDecayInterval)
        {
            pGameState->heat--;
        }
    }

    ProcessRandomEvents(pGameState);

    pGameState->market.fluctuationTimer--;
    if (pGameState->market.fluctuationTimer <= 0)
    {
        pGameState->market.fluctuationTimer = kMarketFluctuationInterval;
        for (int i = 0; i < static_cast<int>(MarketItemType::COUNT); i++)
        {
            float change = static_cast<float>(GameRandomRange(pGameState, -kMarketFluctuationRange, kMarketFluctuationRange)) / 100.0f;
            pGameState->market.priceMultipliers[i] *= (1.0f + change);
            if (pGameState->market.priceMultipliers[i] < 0.5f)
            {
                pGameState->market.priceMultipliers[i] = 0.5f;
            }
            if (pGameState->market.priceMultipliers[i] > 2.0f)
            {
                pGameState->market.priceMultipliers[i] = 2.0f;
            }
        }
    }

    pGameState->maxJobSlots = GetMaxJobSlots(pGameState);
    pGameState->maxPersonnelSlots = GetMaxPersonnelSlots(pGameState);

    int jobCount = GetJobCount();
    for (int i = 0; i < jobCount; i++)
    {
        const JobDef& job = GetJobDef(i);
        if (true == pGameState->jobUnlocked[i])
        {
            continue;
        }

        if (JOB_ID_FORUM_TIP_FOLLOWUP == i)
        {
            if (true == pGameState->pendingForumTip && false == pGameState->jobUnlocked[i])
            {
                pGameState->jobUnlocked[i] = true;
                AddEventLog(pGameState, L"Forum tip lead available in Jobs panel.", Colors::Yellow);
            }
            continue;
        }

        // Contractor jobs are unlocked only through military service completion
        if (true == IsContractorJob(i))
        {
            continue;
        }

        if (true == job.isLegitimate && false == pGameState->inPrestigeMode)
        {
            continue;
        }
        if (false == job.isLegitimate && true == pGameState->inPrestigeMode)
        {
            continue;
        }

        bool tierOk = IsJobTierUnlocked(pGameState, job.tier);
        bool skillsOk = MeetsSkillRequirements(pGameState, job);
        bool docOk = (DocId::COUNT == job.requiredDoc) || true == pGameState->docsRead[static_cast<int>(job.requiredDoc)];
        bool prereqOk = (job.prerequisiteJobId < 0) || (pGameState->jobCompletionCounts[job.prerequisiteJobId] >= job.prerequisiteJobCount);
        if (true == tierOk && true == skillsOk && true == docOk && true == prereqOk)
        {
            pGameState->jobUnlocked[i] = true;
            auto unlockMsg = std::make_unique<wchar_t[]>(BUF_XLARGE);
            StringCchPrintfW(unlockMsg.get(), BUF_XLARGE, L"New job unlocked: %s", job.name);
            AddEventLog(pGameState, unlockMsg.get(), Colors::Yellow);
            AddToast(pGameState, unlockMsg.get(), Colors::Yellow);

            if (true == pGameState->jobUnlockFromIRC && pGameState->pendingJobUnlockId < 0)
            {
                pGameState->pendingJobUnlockId = i;
            }
        }
    }

    if (pGameState->payoutMultiplierTimer > 0)
    {
        pGameState->payoutMultiplierTimer--;
        if (pGameState->payoutMultiplierTimer <= 0)
        {
            pGameState->payoutMultiplier = 1.0f;
        }
    }
    if (pGameState->exploitPriceTimer > 0)
    {
        pGameState->exploitPriceTimer--;
        if (pGameState->exploitPriceTimer <= 0)
        {
            pGameState->exploitPriceMultiplier = 1.0f;
        }
    }
    if (pGameState->cryptoPayoutTimer > 0)
    {
        pGameState->cryptoPayoutTimer--;
        if (pGameState->cryptoPayoutTimer <= 0)
        {
            pGameState->cryptoPayoutMultiplier = 1.0f;
        }
    }
    if (pGameState->marketPriceBoostTimer > 0)
    {
        pGameState->marketPriceBoostTimer--;
        if (pGameState->marketPriceBoostTimer <= 0)
        {
            pGameState->marketPriceBoost = 1.0f;
        }
    }

    // Documentation study timer
    if (true == pGameState->docStudyActive)
    {
        pGameState->docStudyRemaining--;
        if (pGameState->docStudyRemaining <= 0)
        {
            pGameState->docStudyActive = false;
            int docIdx = static_cast<int>(pGameState->currentDocStudy);
            pGameState->docStudyPasses[docIdx]++;
            int pass = pGameState->docStudyPasses[docIdx];

            if (1 == pass)
            {
                pGameState->docsRead[docIdx] = true;
            }

            const DocStudyDef& doc = GetDocDef(docIdx);

            // XP formula: starts at 5%, decreases gradually per pass
            float xpMult = kDocStudyBaseXPMult * (1.0f - (pass * kDocStudyXPDecayPerPass));
            if (xpMult < kDocStudyMinXPMult)
            {
                xpMult = kDocStudyMinXPMult;
            }

            for (int i = 0; i < doc.numSkillBonuses; i++)
            {
                int xp = static_cast<int>(doc.skillBonus[i].xpGain * xpMult);
                if (xp < 1)
                {
                    xp = 1;
                }
                pGameState->skills[doc.skillBonus[i].skill].xp += xp;
            }

            // Determine mastery tier name
            const wchar_t* passName = L"Novice";
            if (pass >= DOC_TIER_MASTER)
            {
                passName = L"Master";
            }
            else if (pass >= DOC_TIER_EXPERT)
            {
                passName = L"Expert";
            }
            else if (pass >= DOC_TIER_PROFICIENT)
            {
                passName = L"Proficient";
            }
            else if (pass >= DOC_TIER_COMPETENT)
            {
                passName = L"Competent";
            }
            else if (pass >= DOC_TIER_FAMILIAR)
            {
                passName = L"Familiar";
            }
            else if (pass >= DOC_TIER_BEGINNER)
            {
                passName = L"Beginner";
            }

            auto studyMsg = std::make_unique<wchar_t[]>(BUF_XLARGE);
            int masteryPercent = (pass * 100) / MAX_DOC_PASSES;
            StringCchPrintfW(studyMsg.get(), BUF_XLARGE, L"Finished studying: %s (Pass %d/%d - %s %d%%)", doc.name, pass, MAX_DOC_PASSES, passName, masteryPercent);
            AddEventLog(pGameState, studyMsg.get(), Colors::Purple);
            AddToast(pGameState, studyMsg.get(), Colors::Purple);
        }
    }

    // Mentorship timer decay
    if (pGameState->skillXPMultiplierTimer > 0)
    {
        pGameState->skillXPMultiplierTimer--;
        if (pGameState->skillXPMultiplierTimer <= 0)
        {
            pGameState->skillXPMultiplier = 1.0f;
        }
    }

    // Boss cooldown decay
    for (int i = 0; i < NUM_BOSS_HACKS; i++)
    {
        if (pGameState->bossAttemptCooldown[i] > 0)
        {
            pGameState->bossAttemptCooldown[i]--;
        }
    }

    // Intel market refresh
    pGameState->intelMarketRefreshTimer--;
    if (pGameState->intelMarketRefreshTimer <= 0)
    {
        pGameState->intelMarketRefreshTimer = INTEL_REFRESH_INTERVAL;
        RefreshIntelMarketStock(pGameState);
    }

    // Game hack ban timer decay
    if (pGameState->gameHackBanTimer > 0)
    {
        pGameState->gameHackBanTimer--;
    }

    // Clear IRC unlock flag at end of tick
    pGameState->jobUnlockFromIRC = false;

    pGameState->cursorBlinkTimer++;
    if (pGameState->cursorBlinkTimer >= kCursorBlinkInterval)
    {
        pGameState->cursorBlinkTimer = 0;
        pGameState->cursorVisible = !pGameState->cursorVisible;
    }

    pGameState->autosaveCounter++;
    if (pGameState->autosaveCounter >= AUTOSAVE_INTERVAL)
    {
        pGameState->autosaveCounter = 0;
        SaveGame(pGameState);
    }

    // Heat reaching max triggers federal seizure
    if (pGameState->heat >= HEAT_MAX && false == pGameState->jailed)
    {
        pGameState->jailed = true;
        pGameState->jailEndTimestamp = static_cast<int64_t>(time(NULL)) + JAIL_DURATION_SECONDS;
        pGameState->triggerJail = true;

        // Cancel all active jobs
        for (int i = 0; i < MAX_ACTIVE_JOBS; i++)
        {
            if (true == pGameState->activeJobs[i].active)
            {
                if (pGameState->activeJobs[i].assignedPersonnelId >= 0 &&
                    pGameState->activeJobs[i].assignedPersonnelId < MAX_PERSONNEL)
                {
                    pGameState->personnel[pGameState->activeJobs[i].assignedPersonnelId].assignedJobSlot = -1;
                }
                pGameState->activeJobs[i].active = false;
                pGameState->activeJobs[i].assignedPersonnelId = -1;
            }
        }

        AddEventLog(pGameState, L"FEDERAL SEIZURE! Your operation has been shut down by the FBI.", Colors::Red);
        AddEventLog(pGameState, L"All operations suspended for 24 hours.", Colors::Red);
        AddToast(pGameState, L"YOU HAVE BEEN SEIZED BY THE FEDS!", Colors::Red);
    }
}
