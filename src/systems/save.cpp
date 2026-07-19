#include "../core/game.h"
#include "../ui/colors.h"
#include "../systems/offline.h"
#include <shlobj.h>
#include <strsafe.h>
#include <memory>
#include <cstdio>

// ============================================================================
// Save/Load System - Binary serialization to %APPDATA%\ShadowNet\save.dat
// ============================================================================

static const uint32_t SAVE_MAGIC = 0x534E4554; // "SNET"
static const uint32_t SAVE_VERSION = 6;

constexpr uint32_t kMinSaveVersion = 1;
constexpr uint32_t kMaxSaveVersion = 6;
constexpr int kJobTrackingArraySize = 128;
constexpr int kSkillCheckpointLevel75 = 75;
constexpr int kSkillCheckpointLevel50 = 50;
constexpr int kSkillCheckpointLevel25 = 25;

// ============================================================================
// Load-time validation
//
// LoadGame reads a raw binary dump of GameState from disk. The file is outside
// the game's trust boundary (a user or another process can edit save.dat), so
// every value that is later used as an array index, loop count, or bit-shift
// amount must be clamped to a safe range before it is trusted. Without this,
// a crafted save produces out-of-bounds reads/writes (several firing on load
// via ProcessOfflineProgress). This runs after all freads, before any field is
// used.
// ============================================================================

template <typename T>
static void ClampValue(T& v, T lo, T hi)
{
    if (v < lo) { v = lo; }
    else if (v > hi) { v = hi; }
}

static void SanitizeLoadedState(GameState* gs)
{
    const int jobCount = GetJobCount();

    // Core scalars
    ClampValue<int64_t>(gs->cash, 0, INT64_MAX);
    ClampValue(gs->reputation, 0, 1000000000);
    ClampValue(gs->heat, 0, HEAT_MAX);
    ClampValue(gs->prestigeLevel, 0, 100000);
    if (gs->totalTicksPlayed < 0) { gs->totalTicksPlayed = 0; }

    // Skills
    for (int i = 0; i < NUM_SKILLS; i++)
    {
        ClampValue(gs->skills[i].level, 0, 100);
        if (gs->skills[i].xp < 0) { gs->skills[i].xp = 0; }
    }

    // Active jobs: invalid job/personnel indices are the highest-severity risk
    for (int i = 0; i < MAX_ACTIVE_JOBS; i++)
    {
        ActiveJob& aj = gs->activeJobs[i];
        if (false == aj.active)
        {
            aj.assignedPersonnelId = -1;
            continue;
        }
        if (aj.jobDefId < 0 || aj.jobDefId >= jobCount ||
            aj.assignedPersonnelId < -1 || aj.assignedPersonnelId >= MAX_PERSONNEL)
        {
            aj.active = false;
            aj.assignedPersonnelId = -1;
            continue;
        }
        if (aj.remainingSeconds < 0) { aj.remainingSeconds = 0; }
        if (aj.totalSeconds < 1) { aj.totalSeconds = 1; }
    }
    ClampValue(gs->maxJobSlots, 1, MAX_ACTIVE_JOBS);

    // Job completion tracking (fixed 128-wide arrays; nothing to clamp on the
    // array itself, but keep counts non-negative)
    for (int i = 0; i < 128; i++)
    {
        if (gs->jobCompletionCounts[i] < 0) { gs->jobCompletionCounts[i] = 0; }
    }

    // Personnel: type, slot, and an unterminated name are all reachable OOB
    for (int i = 0; i < MAX_PERSONNEL; i++)
    {
        PersonnelMember& pm = gs->personnel[i];
        pm.name[_countof(pm.name) - 1] = L'\0'; // force NUL-termination
        if (false == pm.active)
        {
            pm.assignedJobSlot = -1;
            continue;
        }
        if (static_cast<int>(pm.type) < 0 || static_cast<int>(pm.type) >= static_cast<int>(PersonnelType::COUNT))
        {
            pm.type = PersonnelType::ScriptKiddie;
        }
        if (pm.assignedJobSlot < -1 || pm.assignedJobSlot >= MAX_ACTIVE_JOBS)
        {
            pm.assignedJobSlot = -1;
        }
        ClampValue(pm.level, 1, 1000000);
        ClampValue(pm.loyalty, 0, 100);
        if (pm.arrestTimer < 0) { pm.arrestTimer = 0; }
    }
    ClampValue(gs->maxPersonnelSlots, 1, MAX_PERSONNEL);
    ClampValue(gs->purchasedPersonnelSlots, 0, MAX_PERSONNEL);

    // Inventory counts
    if (gs->inventory.burnerPhones < 0) { gs->inventory.burnerPhones = 0; }
    if (gs->inventory.dedicatedServers < 0) { gs->inventory.dedicatedServers = 0; }
    if (gs->inventory.bulletproofVPS < 0) { gs->inventory.bulletproofVPS = 0; }
    if (gs->inventory.dataDumps < 0) { gs->inventory.dataDumps = 0; }
    if (gs->inventory.exploits < 0) { gs->inventory.exploits = 0; }
    if (gs->inventory.credentials < 0) { gs->inventory.credentials = 0; }
    if (gs->inventory.malwareKits < 0) { gs->inventory.malwareKits = 0; }
    if (gs->inventory.forgedDocs < 0) { gs->inventory.forgedDocs = 0; }

    // Documentation study
    if (static_cast<int>(gs->currentDocStudy) < 0 ||
        static_cast<int>(gs->currentDocStudy) >= static_cast<int>(DocId::COUNT))
    {
        gs->currentDocStudy = static_cast<DocId>(0);
        gs->docStudyActive = false;
    }
    if (gs->docStudyRemaining < 0) { gs->docStudyRemaining = 0; }
    if (gs->docStudyTotal < 0) { gs->docStudyTotal = 0; }
    for (int i = 0; i < MAX_DOCUMENTATION; i++)
    {
        ClampValue(gs->docStudyPasses[i], 0, MAX_DOC_PASSES);
    }

    // Skill checkpoints
    for (int i = 0; i < NUM_SKILLS; i++)
    {
        ClampValue(gs->skillCheckpoints[i], 0, 100);
    }

    // Intel arrays
    for (int i = 0; i < static_cast<int>(IntelItemType::COUNT); i++)
    {
        if (gs->intelInventory[i] < 0) { gs->intelInventory[i] = 0; }
        if (gs->intelMarketStock[i] < 0) { gs->intelMarketStock[i] = 0; }
    }

    // Boss state
    for (int i = 0; i < NUM_BOSS_HACKS; i++)
    {
        if (gs->bossAttemptCooldown[i] < 0) { gs->bossAttemptCooldown[i] = 0; }
    }

    // Timers / misc counters
    if (gs->gameHackBanTimer < 0) { gs->gameHackBanTimer = 0; }
    if (gs->consecutiveJobsNoProtection < 0) { gs->consecutiveJobsNoProtection = 0; }
    if (gs->scamHireCount < 0) { gs->scamHireCount = 0; }
    if (gs->militaryDaysCompleted < 0) { gs->militaryDaysCompleted = 0; }
    if (gs->militaryPayTimer < 0) { gs->militaryPayTimer = 0; }
    if (gs->militaryBusyworkTimer < 0) { gs->militaryBusyworkTimer = 0; }

    // Military specialty is used directly as a skill index
    if (static_cast<int>(gs->militarySpecialty) < 0 ||
        static_cast<int>(gs->militarySpecialty) >= NUM_SKILLS)
    {
        gs->militarySpecialty = SKILL_WINDOWS_EXPLOITATION;
    }
    if (static_cast<int>(gs->militaryPhase) < static_cast<int>(MilitaryPhase::None) ||
        static_cast<int>(gs->militaryPhase) > static_cast<int>(MilitaryPhase::Completed))
    {
        gs->militaryPhase = MilitaryPhase::None;
    }

    // UI state used as indices
    if (static_cast<int>(gs->currentTab) < 0 || static_cast<int>(gs->currentTab) >= static_cast<int>(Tab::COUNT))
    {
        gs->currentTab = Tab::Dashboard;
    }
    ClampValue(gs->jobFilterTier, 0, 4);
    if (gs->jobScrollOffset < 0) { gs->jobScrollOffset = 0; }
    if (gs->skillScrollOffset < 0) { gs->skillScrollOffset = 0; }
    if (gs->personnelScrollOffset < 0) { gs->personnelScrollOffset = 0; }
    if (gs->marketScrollOffset < 0) { gs->marketScrollOffset = 0; }
    if (gs->dashboardScrollOffset < 0) { gs->dashboardScrollOffset = 0; }
    if (gs->militaryScrollOffset < 0) { gs->militaryScrollOffset = 0; }
}

static bool GetSavePath(wchar_t* path, int maxLen)
{
    auto appData = std::make_unique<wchar_t[]>(MAX_PATH);
    if (FAILED(SHGetFolderPathW(NULL, CSIDL_APPDATA, NULL, 0, appData.get())))
    {
        return false;
    }

    StringCchPrintfW(path, maxLen, L"%s\\ShadowNet", appData.get());
    CreateDirectoryW(path, NULL);
    StringCchPrintfW(path, maxLen, L"%s\\ShadowNet\\save.dat", appData.get());
    return true;
}

bool SaveGame(const GameState* gs)
{
    auto path = std::make_unique<wchar_t[]>(MAX_PATH);
    if (false == GetSavePath(path.get(), MAX_PATH))
    {
        return false;
    }

    FILE* f = nullptr;
    if (0 != _wfopen_s(&f, path.get(), L"wb") || nullptr == f)
    {
        return false;
    }

    // Header
    fwrite(&SAVE_MAGIC, sizeof(uint32_t), 1, f);
    fwrite(&SAVE_VERSION, sizeof(uint32_t), 1, f);

    // Timestamp
    int64_t now = static_cast<int64_t>(time(NULL));
    fwrite(&now, sizeof(int64_t), 1, f);

    // Core state
    fwrite(&gs->cash, sizeof(int64_t), 1, f);
    fwrite(&gs->reputation, sizeof(int), 1, f);
    fwrite(&gs->heat, sizeof(int), 1, f);
    fwrite(&gs->prestigeLevel, sizeof(int), 1, f);
    fwrite(&gs->inPrestigeMode, sizeof(bool), 1, f);
    fwrite(&gs->totalTicksPlayed, sizeof(int64_t), 1, f);

    // Skills
    fwrite(gs->skills, sizeof(SkillState), NUM_SKILLS, f);

    // Active jobs
    fwrite(gs->activeJobs, sizeof(ActiveJob), MAX_ACTIVE_JOBS, f);
    fwrite(&gs->maxJobSlots, sizeof(int), 1, f);

    // Job completion tracking
    fwrite(gs->jobCompletionCounts, sizeof(int), kJobTrackingArraySize, f);
    fwrite(gs->jobUnlocked, sizeof(bool), kJobTrackingArraySize, f);

    // Personnel
    fwrite(gs->personnel, sizeof(PersonnelMember), MAX_PERSONNEL, f);
    fwrite(&gs->maxPersonnelSlots, sizeof(int), 1, f);

    // Inventory
    fwrite(&gs->inventory, sizeof(Inventory), 1, f);

    // Market
    fwrite(&gs->market, sizeof(MarketState), 1, f);

    // Event effects
    fwrite(&gs->payoutMultiplier, sizeof(float), 1, f);
    fwrite(&gs->payoutMultiplierTimer, sizeof(int), 1, f);
    fwrite(&gs->exploitPriceMultiplier, sizeof(float), 1, f);
    fwrite(&gs->exploitPriceTimer, sizeof(int), 1, f);
    fwrite(&gs->cryptoPayoutMultiplier, sizeof(float), 1, f);
    fwrite(&gs->cryptoPayoutTimer, sizeof(int), 1, f);
    fwrite(&gs->marketPriceBoost, sizeof(float), 1, f);
    fwrite(&gs->marketPriceBoostTimer, sizeof(int), 1, f);

    // Stats
    fwrite(&gs->totalEarned, sizeof(int64_t), 1, f);
    fwrite(&gs->totalJobsCompleted, sizeof(int), 1, f);

    // RNG state
    fwrite(&gs->rngState, sizeof(uint32_t), 1, f);

    // V2 fields
    fwrite(gs->docsRead, sizeof(bool), static_cast<int>(DocId::COUNT), f);
    fwrite(&gs->docStudyActive, sizeof(bool), 1, f);
    fwrite(&gs->currentDocStudy, sizeof(DocId), 1, f);
    fwrite(&gs->docStudyRemaining, sizeof(int), 1, f);
    fwrite(&gs->docStudyTotal, sizeof(int), 1, f);
    fwrite(&gs->purchasedPersonnelSlots, sizeof(int), 1, f);
    fwrite(&gs->skillXPMultiplier, sizeof(float), 1, f);
    fwrite(&gs->skillXPMultiplierTimer, sizeof(int), 1, f);

    // V3 fields
    fwrite(gs->tierBossCompleted, sizeof(bool), NUM_BOSS_HACKS, f);
    fwrite(gs->bossAttemptCooldown, sizeof(int), NUM_BOSS_HACKS, f);
    fwrite(gs->bossPrepDone, sizeof(bool), NUM_BOSS_HACKS * MAX_BOSS_PREP, f);

    // V4 fields
    fwrite(&gs->pendingForumTip, sizeof(bool), 1, f);
    fwrite(&gs->forumTipPayout, sizeof(int64_t), 1, f);
    fwrite(gs->docStudyPasses, sizeof(int), MAX_DOCUMENTATION, f);
    fwrite(gs->skillCheckpoints, sizeof(int), NUM_SKILLS, f);
    fwrite(gs->intelInventory, sizeof(int), static_cast<int>(IntelItemType::COUNT), f);
    fwrite(gs->intelMarketStock, sizeof(int), static_cast<int>(IntelItemType::COUNT), f);
    fwrite(gs->intelPriceMultipliers, sizeof(float), static_cast<int>(IntelItemType::COUNT), f);
    fwrite(&gs->intelMarketRefreshTimer, sizeof(int), 1, f);
    fwrite(&gs->gameHackBanTimer, sizeof(int), 1, f);
    fwrite(&gs->consecutiveJobsNoProtection, sizeof(int), 1, f);
    fwrite(&gs->heatCompoundMultiplier, sizeof(float), 1, f);
    fwrite(&gs->scamHireCount, sizeof(int), 1, f);

    // V5 fields - jail system
    fwrite(&gs->jailed, sizeof(bool), 1, f);
    fwrite(&gs->jailEndTimestamp, sizeof(int64_t), 1, f);

    // V6 fields - military cyber route
    fwrite(&gs->militaryPhase, sizeof(MilitaryPhase), 1, f);
    fwrite(&gs->militaryEnlistTimestamp, sizeof(int64_t), 1, f);
    fwrite(&gs->militaryDaysCompleted, sizeof(int), 1, f);
    fwrite(&gs->militaryDayStartTimestamp, sizeof(int64_t), 1, f);
    fwrite(&gs->militarySpecialty, sizeof(SkillId), 1, f);
    fwrite(&gs->militaryPayTimer, sizeof(int), 1, f);
    fwrite(&gs->militaryTotalPay, sizeof(int64_t), 1, f);
    fwrite(&gs->militaryEnlistAvailable, sizeof(bool), 1, f);
    fwrite(&gs->cyberVeteran, sizeof(bool), 1, f);
    fwrite(&gs->militaryBusyworkTimer, sizeof(int), 1, f);

    fclose(f);
    return true;
}

bool LoadGame(GameState* gs)
{
    auto path = std::make_unique<wchar_t[]>(MAX_PATH);
    if (false == GetSavePath(path.get(), MAX_PATH))
    {
        return false;
    }

    FILE* f = nullptr;
    if (0 != _wfopen_s(&f, path.get(), L"rb") || nullptr == f)
    {
        return false;
    }

    // Header
    uint32_t magic = 0, version = 0;
    if (1 != fread(&magic, sizeof(uint32_t), 1, f) ||
        1 != fread(&version, sizeof(uint32_t), 1, f))
    {
        fclose(f);
        return false;
    }

    if (SAVE_MAGIC != magic || version < kMinSaveVersion || version > kMaxSaveVersion)
    {
        fclose(f);
        return false;
    }

    // Timestamp (for offline progress)
    int64_t saveTime = 0;
    if (1 != fread(&saveTime, sizeof(int64_t), 1, f))
    {
        fclose(f);
        return false;
    }

    // Core state
    fread(&gs->cash, sizeof(int64_t), 1, f);
    fread(&gs->reputation, sizeof(int), 1, f);
    fread(&gs->heat, sizeof(int), 1, f);
    fread(&gs->prestigeLevel, sizeof(int), 1, f);
    fread(&gs->inPrestigeMode, sizeof(bool), 1, f);
    fread(&gs->totalTicksPlayed, sizeof(int64_t), 1, f);

    // Skills
    fread(gs->skills, sizeof(SkillState), NUM_SKILLS, f);

    // Active jobs
    fread(gs->activeJobs, sizeof(ActiveJob), MAX_ACTIVE_JOBS, f);
    fread(&gs->maxJobSlots, sizeof(int), 1, f);

    // Job completion tracking
    fread(gs->jobCompletionCounts, sizeof(int), kJobTrackingArraySize, f);
    fread(gs->jobUnlocked, sizeof(bool), kJobTrackingArraySize, f);

    // Personnel
    fread(gs->personnel, sizeof(PersonnelMember), MAX_PERSONNEL, f);
    fread(&gs->maxPersonnelSlots, sizeof(int), 1, f);

    // Inventory
    fread(&gs->inventory, sizeof(Inventory), 1, f);

    // Market
    fread(&gs->market, sizeof(MarketState), 1, f);

    // Event effects
    fread(&gs->payoutMultiplier, sizeof(float), 1, f);
    fread(&gs->payoutMultiplierTimer, sizeof(int), 1, f);
    fread(&gs->exploitPriceMultiplier, sizeof(float), 1, f);
    fread(&gs->exploitPriceTimer, sizeof(int), 1, f);
    fread(&gs->cryptoPayoutMultiplier, sizeof(float), 1, f);
    fread(&gs->cryptoPayoutTimer, sizeof(int), 1, f);
    fread(&gs->marketPriceBoost, sizeof(float), 1, f);
    fread(&gs->marketPriceBoostTimer, sizeof(int), 1, f);

    // Stats
    fread(&gs->totalEarned, sizeof(int64_t), 1, f);
    fread(&gs->totalJobsCompleted, sizeof(int), 1, f);

    // RNG state
    fread(&gs->rngState, sizeof(uint32_t), 1, f);

    // V2 fields
    if (version >= 2)
    {
        fread(gs->docsRead, sizeof(bool), static_cast<int>(DocId::COUNT), f);
        fread(&gs->docStudyActive, sizeof(bool), 1, f);
        fread(&gs->currentDocStudy, sizeof(DocId), 1, f);
        fread(&gs->docStudyRemaining, sizeof(int), 1, f);
        fread(&gs->docStudyTotal, sizeof(int), 1, f);
        fread(&gs->purchasedPersonnelSlots, sizeof(int), 1, f);
        fread(&gs->skillXPMultiplier, sizeof(float), 1, f);
        fread(&gs->skillXPMultiplierTimer, sizeof(int), 1, f);
    }
    else
    {
        ZeroMemory(gs->docsRead, sizeof(gs->docsRead));
        gs->docStudyActive = false;
        gs->purchasedPersonnelSlots = 0;
        gs->skillXPMultiplier = 1.0f;
        gs->skillXPMultiplierTimer = 0;
    }

    // V3 fields
    if (version >= 3)
    {
        fread(gs->tierBossCompleted, sizeof(bool), NUM_BOSS_HACKS, f);
        fread(gs->bossAttemptCooldown, sizeof(int), NUM_BOSS_HACKS, f);
        fread(gs->bossPrepDone, sizeof(bool), NUM_BOSS_HACKS * MAX_BOSS_PREP, f);
    }
    else
    {
        ZeroMemory(gs->tierBossCompleted, sizeof(gs->tierBossCompleted));
        ZeroMemory(gs->bossAttemptCooldown, sizeof(gs->bossAttemptCooldown));
        ZeroMemory(gs->bossPrepDone, sizeof(gs->bossPrepDone));
    }

    // V4 fields
    if (version >= 4)
    {
        fread(&gs->pendingForumTip, sizeof(bool), 1, f);
        fread(&gs->forumTipPayout, sizeof(int64_t), 1, f);
        fread(gs->docStudyPasses, sizeof(int), MAX_DOCUMENTATION, f);
        fread(gs->skillCheckpoints, sizeof(int), NUM_SKILLS, f);
        fread(gs->intelInventory, sizeof(int), static_cast<int>(IntelItemType::COUNT), f);
        fread(gs->intelMarketStock, sizeof(int), static_cast<int>(IntelItemType::COUNT), f);
        fread(gs->intelPriceMultipliers, sizeof(float), static_cast<int>(IntelItemType::COUNT), f);
        fread(&gs->intelMarketRefreshTimer, sizeof(int), 1, f);
        fread(&gs->gameHackBanTimer, sizeof(int), 1, f);
        fread(&gs->consecutiveJobsNoProtection, sizeof(int), 1, f);
        fread(&gs->heatCompoundMultiplier, sizeof(float), 1, f);
        fread(&gs->scamHireCount, sizeof(int), 1, f);
    }
    else
    {
        gs->pendingForumTip = false;
        gs->forumTipPayout = 0;
        ZeroMemory(gs->docStudyPasses, sizeof(gs->docStudyPasses));
        ZeroMemory(gs->skillCheckpoints, sizeof(gs->skillCheckpoints));
        ZeroMemory(gs->intelInventory, sizeof(gs->intelInventory));
        ZeroMemory(gs->intelMarketStock, sizeof(gs->intelMarketStock));
        for (int i = 0; i < static_cast<int>(IntelItemType::COUNT); i++)
        {
            gs->intelPriceMultipliers[i] = 1.0f;
        }
        gs->intelMarketRefreshTimer = 1;
        gs->gameHackBanTimer = 0;
        gs->consecutiveJobsNoProtection = 0;
        gs->heatCompoundMultiplier = 1.0f;
        gs->scamHireCount = 0;
        // Derive doc passes from existing docsRead for backward compat
        for (int i = 0; i < static_cast<int>(DocId::COUNT); i++)
        {
            gs->docStudyPasses[i] = (true == gs->docsRead[i]) ? 1 : 0;
        }
        // Derive skill checkpoints from existing levels
        for (int i = 0; i < NUM_SKILLS; i++)
        {
            if (gs->skills[i].level >= kSkillCheckpointLevel75)
            {
                gs->skillCheckpoints[i] = kSkillCheckpointLevel75;
            }
            else if (gs->skills[i].level >= kSkillCheckpointLevel50)
            {
                gs->skillCheckpoints[i] = kSkillCheckpointLevel50;
            }
            else if (gs->skills[i].level >= kSkillCheckpointLevel25)
            {
                gs->skillCheckpoints[i] = kSkillCheckpointLevel25;
            }
            else
            {
                gs->skillCheckpoints[i] = 0;
            }
        }
    }

    // V5 fields - jail system
    if (version >= 5)
    {
        fread(&gs->jailed, sizeof(bool), 1, f);
        fread(&gs->jailEndTimestamp, sizeof(int64_t), 1, f);

        // Auto-release if jail time has expired
        int64_t now = static_cast<int64_t>(time(NULL));
        if (true == gs->jailed && now >= gs->jailEndTimestamp)
        {
            gs->jailed = false;
            gs->jailEndTimestamp = 0;
        }
        // If still jailed, trigger the jail window on load
        if (true == gs->jailed)
        {
            gs->triggerJail = true;
        }
    }
    else
    {
        gs->jailed = false;
        gs->jailEndTimestamp = 0;
    }

    // V6 fields - military cyber route
    if (version >= 6)
    {
        fread(&gs->militaryPhase, sizeof(MilitaryPhase), 1, f);
        fread(&gs->militaryEnlistTimestamp, sizeof(int64_t), 1, f);
        fread(&gs->militaryDaysCompleted, sizeof(int), 1, f);
        fread(&gs->militaryDayStartTimestamp, sizeof(int64_t), 1, f);
        fread(&gs->militarySpecialty, sizeof(SkillId), 1, f);
        fread(&gs->militaryPayTimer, sizeof(int), 1, f);
        fread(&gs->militaryTotalPay, sizeof(int64_t), 1, f);
        fread(&gs->militaryEnlistAvailable, sizeof(bool), 1, f);
        fread(&gs->cyberVeteran, sizeof(bool), 1, f);
        fread(&gs->militaryBusyworkTimer, sizeof(int), 1, f);
    }
    else
    {
        gs->militaryPhase = MilitaryPhase::None;
        gs->militaryEnlistTimestamp = 0;
        gs->militaryDaysCompleted = 0;
        gs->militaryDayStartTimestamp = 0;
        gs->militarySpecialty = SKILL_WINDOWS_EXPLOITATION;
        gs->militaryPayTimer = 0;
        gs->militaryTotalPay = 0;
        gs->militaryEnlistAvailable = false;
        gs->cyberVeteran = false;
        gs->militaryBusyworkTimer = 0;
    }

    // Always init transient fields
    gs->intelScrollOffset = 0;
    gs->militaryScrollOffset = 0;
    gs->pendingJobUnlockId = -1;
    gs->jobUnlockFromIRC = false;
    gs->triggerJail = gs->jailed; // re-show jail on load if still jailed

    fclose(f);

    // Validate/clamp every loaded field before any of it is used as an index.
    SanitizeLoadedState(gs);

    // Calculate offline progress
    int64_t now = static_cast<int64_t>(time(NULL));
    int64_t elapsed = now - saveTime;
    if (0 > elapsed)
    {
        elapsed = 0;
    }
    if (elapsed > OFFLINE_CAP_SECONDS)
    {
        elapsed = OFFLINE_CAP_SECONDS;
    }

    gs->lastSaveTimestamp = saveTime;

    if (0 < elapsed)
    {
        ProcessOfflineProgress(gs, elapsed);
    }

    return true;
}

bool DeleteSaveFile()
{
    auto path = std::make_unique<wchar_t[]>(MAX_PATH);
    if (false == GetSavePath(path.get(), MAX_PATH))
    {
        return false;
    }
    return 0 != DeleteFileW(path.get());
}
