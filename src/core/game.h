#pragma once
#include <windows.h>
#include <string>
#include <vector>
#include <array>
#include <memory>
#include <cstdint>
#include <ctime>

// ============================================================================
// Constants
// ============================================================================

constexpr int MAX_ACTIVE_JOBS = 10;
constexpr int MAX_PERSONNEL = 15;
constexpr int MAX_EVENT_LOG = 100;
constexpr int MAX_MARKET_HISTORY = 10;
constexpr int OFFLINE_CAP_SECONDS = 8 * 3600; // 8 hours
constexpr int AUTOSAVE_INTERVAL = 60;          // ticks

constexpr int NUM_SKILLS = 17; // 15 core + Art + AI/ML

// Buffer size constants (for heap-allocated wchar_t buffers)
constexpr size_t BUF_TINY = 16;
constexpr size_t BUF_SMALL = 32;
constexpr size_t BUF_MEDIUM = 64;
constexpr size_t BUF_LARGE = 128;
constexpr size_t BUF_XLARGE = 256;
constexpr size_t BUF_LINE = 280;

// Reputation thresholds for tier unlocks
constexpr int REP_TIER3_UNLOCK = 200;
constexpr int REP_TIER2_UNLOCK = 750;
constexpr int REP_TIER1_UNLOCK = 3000;
constexpr int REP_SHADOW_BROKER = 10000;

// Heat thresholds
constexpr int HEAT_MAX = 100;
constexpr int HEAT_WARNING = 25;
constexpr int HEAT_DANGER = 50;
constexpr int HEAT_CRITICAL = 75;
constexpr int HEAT_EXTREME = 90;
constexpr int HEAT_DECAY_INTERVAL = 30; // seconds between heat decay ticks

// Prestige requirements
constexpr int PRESTIGE_REP_REQUIRED = 10000;
constexpr int64_t PRESTIGE_CASH_REQUIRED = 5000000;
constexpr int PRESTIGE_TIER1_JOBS_REQUIRED = 5;
constexpr int PRESTIGE_CASH_RATIO = 10;

// Skill indices
enum SkillId : int
{
    SKILL_WINDOWS_EXPLOITATION = 0,
    SKILL_UNIX_LINUX,
    SKILL_WEB_APP_SECURITY,
    SKILL_NETWORK_ENGINEERING,
    SKILL_MOBILE_PLATFORMS,
    SKILL_EMBEDDED_RTOS,
    SKILL_CRYPTOGRAPHY,
    SKILL_SOCIAL_ENGINEERING,
    SKILL_DATABASE_SYSTEMS,
    SKILL_REVERSE_ENGINEERING,
    SKILL_OPSEC,
    SKILL_FINANCIAL_SYSTEMS,
    SKILL_TELECOMMUNICATIONS,
    SKILL_RADIO_FREQUENCY,
    SKILL_FORENSIC_EVASION,
    SKILL_ART,            // Unique: BBS Art
    SKILL_AI_ML,           // Unique: AI/ML for deepfakes
};

inline const wchar_t* SkillNames[NUM_SKILLS] = {
    L"Windows Exploitation",
    L"UNIX/Linux Admin",
    L"Web App Security",
    L"Network Engineering",
    L"Mobile Platforms",
    L"Embedded/RTOS",
    L"Cryptography",
    L"Social Engineering",
    L"Database Systems",
    L"Reverse Engineering",
    L"OPSEC",
    L"Financial Systems",
    L"Telecommunications",
    L"Radio Frequency",
    L"Forensic Evasion",
    L"Art",
    L"AI/ML",
};

// ============================================================================
// Enums
// ============================================================================

enum class Tab : int
{
    Dashboard = 0,
    Jobs,
    Personnel,
    Skills,
    Market,
    Intel,
    Military,
    Prestige,
    COUNT
};

inline const wchar_t* TabNames[] = {
    L"Dashboard", L"Jobs", L"Personnel", L"Skills", L"Market", L"Intel", L"Military", L"Prestige"
};

enum class Tier : int
{
    Four = 4,   // Script Kiddie
    Three = 3,  // Black Hat
    Two = 2,    // Threat Actor
    One = 1,    // APT Operator / Shadow Broker
};

enum class PlayerTitle : int
{
    ScriptKiddie = 0,  // 0-99
    BlackHat,          // 100-499
    ThreatActor,       // 500-1999
    APTOperator,       // 2000-9999
    ShadowBroker,      // 10000+
};

inline const wchar_t* PlayerTitleNames[] = {
    L"Script Kiddie",
    L"Black Hat",
    L"Threat Actor",
    L"APT Operator",
    L"Shadow Broker",
};

enum class MilitaryPhase : int
{
    None = 0,          // Not enlisted
    Training,          // Day 1: cyber skills train to 65%
    SpecialtyPick,     // End of day 1: pick one skill to push to 100%
    ActiveDuty,        // Days 2-4: busywork + specialty training
    Completed          // Service done, unlock contractor tier
};

// ============================================================================
// Parameter Grouping Structs
// ============================================================================

struct RandomRange
{
    int minVal;
    int maxVal;
};

// ============================================================================
// Skill Training Entry
// ============================================================================

struct SkillTraining
{
    SkillId skill;
    int xpGain;
};

// ============================================================================
// Skill Requirement Entry
// ============================================================================

struct SkillRequirement
{
    SkillId skill;
    int minLevel;
};

// ============================================================================
// Documentation System
// ============================================================================

constexpr int MAX_DOCUMENTATION = 9;
constexpr int MAX_DOC_PASSES = 100;  // Each doc requires 100 study passes to master

// Documentation mastery tier thresholds
constexpr int DOC_TIER_BEGINNER = 10;
constexpr int DOC_TIER_FAMILIAR = 25;
constexpr int DOC_TIER_COMPETENT = 40;
constexpr int DOC_TIER_PROFICIENT = 60;
constexpr int DOC_TIER_EXPERT = 80;
constexpr int DOC_TIER_MASTER = 100;

// Jail / Fed Seizure
constexpr int64_t JAIL_DURATION_SECONDS = 86400;  // 1 real day
constexpr int64_t CONTRACTOR_JAIL_DURATION = 172800; // 2 real days

// Military Cyber Route
constexpr int MILITARY_SERVICE_DAYS = 4;
constexpr int64_t MILITARY_DAY_SECONDS = 86400;     // 1 real day per service day
constexpr int MILITARY_SKILL_CAP_TRAINING = 65;      // Train all cyber skills to 65% during day 1
constexpr int MILITARY_SKILL_CAP_SPECIALTY = 100;     // Specialty goes to 100%
constexpr int MILITARY_PAY_INTERVAL = 3323;           // ~55 min (86400s / 26 biweekly periods)
constexpr int64_t MILITARY_BASE_PAY = 2500;           // Per pay period (low, steady)
constexpr int MILITARY_BUSYWORK_INTERVAL = 30;        // Seconds between busywork events
constexpr int MILITARY_BUSYWORK_CHANCE = 90;          // 90% busywork, 10% cyber
constexpr int NUM_BUSYWORK_EVENTS = 20;
constexpr int NUM_CYBER_WORK_EVENTS = 8;

enum class DocId : int
{
    WinAPIDocs = 0,          // Required for Windows malware jobs
    RFCNetworkDocs,          // Required for router hacking, network jobs
    UNIXManPages,            // Required for UNIX hacking, IRC admin
    WebSecurityRefs,         // Required for web exploit jobs, defacement
    ReverseEngineeringManuals, // Required for exploit development
    CryptoProtocolDocs,      // Required for crypto-related jobs
    SocialEngineeringPlaybook, // Required for advanced social engineering
    EmbeddedSystemsReference,  // Required for hardware/SCADA jobs
    EvilNginxDocs,             // EvilNginx reverse proxy phishing
    COUNT
};

inline const wchar_t* DocNames[(int)DocId::COUNT] = {
    L"WinAPI Documentation",
    L"RFC & Networking Docs",
    L"UNIX Man Pages",
    L"Web Security References",
    L"Reverse Engineering Manuals",
    L"Crypto Protocol Docs",
    L"Social Engineering Playbook",
    L"Embedded Systems Reference",
    L"EvilNginx & Reverse Proxy Phishing",
};

struct DocStudyDef
{
    DocId id;
    const wchar_t* name;
    int studyTimeSeconds;
    int64_t cost;                // 0 = free, >0 = must purchase
    SkillTraining skillBonus[2]; // Skills gained on completion
    int numSkillBonuses;
    const wchar_t* description;
};

// ============================================================================
// Boss Hack System (tier gate bosses)
// ============================================================================

constexpr int NUM_BOSS_HACKS = 4;
constexpr int MAX_BOSS_STAGES = 5;
constexpr int MAX_BOSS_PREP = 3;
constexpr int MAX_BOSS_TERMINAL_LINES = 6;

struct BossStage
{
    const wchar_t* name;
    SkillId requiredSkill;
    int difficultyThreshold;
    const wchar_t* terminalLines[MAX_BOSS_TERMINAL_LINES];
    int numTerminalLines;
};

struct BossPrepOption
{
    const wchar_t* name;
    int64_t cost;
    int timeSeconds;
    int bonusPercent;
};

struct BossHackDef
{
    const wchar_t* operationName;
    const wchar_t* targetName;
    Tier gatedTier;
    BossStage stages[MAX_BOSS_STAGES];
    int numStages;
    BossPrepOption prep[MAX_BOSS_PREP];
    int64_t successCashReward;
    int successRepReward;
    int cooldownSeconds;
    const wchar_t* failureMessage;
    const wchar_t* successMessage;
};

// ============================================================================
// Job Definition (static data)
// ============================================================================

struct JobDef
{
    int id;
    const wchar_t* name;
    Tier tier;
    int baseTimeSeconds;
    int64_t basePayoutUsd;
    int repReward;
    int heatGenerated;
    SkillTraining skillsTrained[5];   // Up to 5 skills trained per job
    int numSkillsTrained;
    SkillRequirement skillReqs[4];    // Up to 4 skill requirements
    int numSkillReqs;
    const wchar_t* description;
    bool isLegitimate;                // Prestige mode job
    int prerequisiteJobId;            // -1 = no prerequisite job required
    int prerequisiteJobCount;         // How many completions of prereq needed (default 1)
    DocId requiredDoc;                // DocId::COUNT = no doc required
};

// ============================================================================
// Active Job (runtime)
// ============================================================================

struct ActiveJob
{
    bool active;
    int jobDefId;             // Index into job catalog
    int remainingSeconds;
    int totalSeconds;
    int assignedPersonnelId;  // -1 if manual
};

// ============================================================================
// Personnel Definition
// ============================================================================

enum class PersonnelType : int
{
    ScriptKiddie = 0,
    ForumLurker,
    Phisher,
    MalwareCoder,
    Carder,
    NetworkSpecialist,
    ExploitDeveloper,
    DataBroker,
    MoneyLaunderer,
    OPSECSpecialist,
    ZeroDayResearcher,
    InsiderHandler,
    InfrastructureArchitect,
    COUNT
};

struct PersonnelTypeDef
{
    const wchar_t* name;
    int64_t cost;
    SkillId specialty;
    int requiredRep;
};

struct PersonnelMember
{
    bool active;
    int id;
    wchar_t name[32];
    PersonnelType type;
    int level;
    int loyalty;         // 0-100
    int assignedJobSlot; // -1 if idle
    bool arrested;
    int arrestTimer;     // seconds remaining
};

// ============================================================================
// Market Item
// ============================================================================

enum class MarketItemType : int
{
    DataDump = 0,
    Exploit,
    Credentials,
    MalwareKit,
    ForgedDocs,
    // Buyable tools/infra
    VPNService,
    ProxyChain,
    BurnerPhone,
    HardwareDevKit,
    ZeroDayKit,
    CustomC2,
    DedicatedServer,
    BulletproofVPS,
    TorHiddenService,
    SatelliteUplink,
    COUNT
};

struct MarketItem
{
    MarketItemType type;
    const wchar_t* name;
    int64_t basePrice;
    bool isBuyable;    // true = player buys, false = player sells
    const wchar_t* description;
};

struct MarketState
{
    float priceMultipliers[(int)MarketItemType::COUNT];
    int fluctuationTimer;
};

// ============================================================================
// Intel Market (Underground Data Market)
// ============================================================================

constexpr int INTEL_REFRESH_INTERVAL = 600; // ticks between stock refresh

enum class IntelItemType : int
{
    // Data Dumps (real breaches)
    LinkedInCombo = 0,     // LinkedIn 2021 (700M)
    ParkMobileBreach,      // ParkMobile 2021 (21M)
    NationalPublicData,    // NPD 2024 (2.9B SSN)
    AutoZoneFullz,         // AutoZone 2023
    MGMResortsDump,        // MGM 2023
    TMobileCustomerDB,     // T-Mobile 2021-2023
    LastPassVaultData,     // LastPass 2022
    MOVEitTransferDump,    // MOVEit 2023 (2.5k orgs)
    EquifaxCreditData,     // Equifax 2017 (147M)
    AnthemHealthRecords,   // Anthem 2015 (80M)
    FacebookMetaLeak,      // Facebook 2021 (533M)
    TwitterXUserData,      // Twitter 2023 (200M)
    // Access
    CorporateVPNAccess,    // VPN credentials to companies
    RDPAccess,             // Remote Desktop access
    // Identity
    StolenIdentity,        // Full identity package
    // Tools
    EvilNginxConfig,       // Pre-built phishing configs
    PhishingKit,           // Ready-made phishing pages
    ExploitPack,           // Collection of working exploits
    COUNT
};

constexpr int NUM_INTEL_ITEMS = (int)IntelItemType::COUNT;

struct IntelItemDef
{
    IntelItemType type;
    const wchar_t* name;
    const wchar_t* description;
    int64_t basePrice;
    int repRequired;        // Min rep to see this item
    const wchar_t* category; // Display category
};

// ============================================================================
// Inventory
// ============================================================================

struct Inventory
{
    int dataDumps;
    int exploits;
    int credentials;
    int malwareKits;
    int forgedDocs;
    // Owned tools (boolean flags)
    bool hasVPN;
    bool hasProxyChain;
    int burnerPhones;
    bool hasHardwareDevKit;
    bool hasZeroDayKit;
    bool hasCustomC2;
    int dedicatedServers;     // can have multiple
    int bulletproofVPS;
    bool hasTorHiddenService;
    bool hasSatelliteUplink;
};

// ============================================================================
// Skill State
// ============================================================================

struct SkillState
{
    int level;
    int xp;
};

// XP required for next level — quadratic curve, tuned so T4 completes in ~1 hour
// Levels 0-50: quadratic (XP_BASE_MULTIPLIER * (level+1)^2)
// Levels 51-100: quadratic base * exponential factor (XP_EXPONENTIAL_GROWTH^(level-XP_EXPONENTIAL_THRESHOLD))
constexpr int XP_BASE_MULTIPLIER = 15;
constexpr int XP_EXPONENTIAL_THRESHOLD = 50;
constexpr double XP_EXPONENTIAL_GROWTH = 1.08;

inline int XpForNextLevel(int currentLevel)
{
    int base = XP_BASE_MULTIPLIER * (currentLevel + 1) * (currentLevel + 1);
    if (currentLevel > XP_EXPONENTIAL_THRESHOLD)
    {
        double expFactor = 1.0;
        for (int i = 0; i < currentLevel - XP_EXPONENTIAL_THRESHOLD; i++)
        {
            expFactor *= XP_EXPONENTIAL_GROWTH;
        }
        return static_cast<int>(base * expFactor);
    }
    return base;
}

// ============================================================================
// Event Log Entry
// ============================================================================

struct EventLogEntry
{
    wchar_t text[256];
    COLORREF color;
    int64_t timestamp; // game tick when it occurred
};

// ============================================================================
// Toast Notification
// ============================================================================

struct Toast
{
    bool active;
    wchar_t text[256];
    COLORREF color;
    int remainingMs; // auto-dismiss timer
};

constexpr int MAX_TOASTS = 5;

// ============================================================================
// Random Event Definition
// ============================================================================

enum class RandomEventType : int
{
    ForumTip = 0,
    LawEnforcementInvestigation,
    RivalHackerAttack,
    FBIRaid,
    InterpolDragnet,
    DataBreachInNews,
    ExploitPatchTuesday,
    CryptoMarketCrash,
    CryptoMarketBoom,
    Whistleblower,
    UndergroundAlliance,
    ZeroDayDropped,
    ArrestOfCompetitor,
    JournalistInvestigation,
    FederalIndictment,
    COUNT
};

// ============================================================================
// Main Game State
// ============================================================================

struct GameState
{
    // Core
    int64_t cash;
    int reputation;
    int heat;               // 0-100
    int prestigeLevel;
    bool inPrestigeMode;

    // Timing
    int64_t totalTicksPlayed;
    int64_t lastSaveTimestamp;  // Unix timestamp
    int autosaveCounter;

    // Skills
    SkillState skills[NUM_SKILLS];

    // Active jobs
    ActiveJob activeJobs[MAX_ACTIVE_JOBS];
    int maxJobSlots;

    // Job completion tracking
    int jobCompletionCounts[128]; // indexed by job def id
    bool jobUnlocked[128];

    // Personnel
    PersonnelMember personnel[MAX_PERSONNEL];
    int maxPersonnelSlots;

    // Inventory
    Inventory inventory;

    // Market
    MarketState market;

    // Event log (ring buffer)
    EventLogEntry eventLog[MAX_EVENT_LOG];
    int eventLogHead;
    int eventLogCount;

    // Toasts
    Toast toasts[MAX_TOASTS];

    // UI state
    Tab currentTab;
    int jobFilterTier;     // 0 = all, 1-4 = specific tier
    int jobSortMode;       // 0=payout, 1=time, 2=heat, 3=tier
    int jobScrollOffset;
    int personnelScrollOffset;
    int skillScrollOffset;
    int marketScrollOffset;
    int dashboardScrollOffset;

    // Temporary event effects
    float payoutMultiplier;       // from events like Underground Alliance
    int payoutMultiplierTimer;
    float exploitPriceMultiplier;
    int exploitPriceTimer;
    float cryptoPayoutMultiplier;
    int cryptoPayoutTimer;
    float marketPriceBoost;
    int marketPriceBoostTimer;

    // Statistics
    int64_t totalEarned;
    int totalJobsCompleted;

    // Cursor blink
    bool cursorVisible;
    int cursorBlinkTimer;

    // Random seed state
    uint32_t rngState;

    // Documentation study (Phase 2)
    bool docsRead[(int)DocId::COUNT];
    bool docStudyActive;
    DocId currentDocStudy;
    int docStudyRemaining;
    int docStudyTotal;

    // Personnel slot upgrades (Phase 2)
    int purchasedPersonnelSlots;

    // IRC mentorship bonus (Phase 3)
    float skillXPMultiplier;
    int skillXPMultiplierTimer;

    // Boss hack state
    bool tierBossCompleted[NUM_BOSS_HACKS];
    int bossAttemptCooldown[NUM_BOSS_HACKS];
    bool bossPrepDone[NUM_BOSS_HACKS][MAX_BOSS_PREP];

    // V4 fields

    // Forum tip rework - requires follow-up work
    bool pendingForumTip;
    int64_t forumTipPayout;  // randomized when tip fires

    // Documentation multi-pass learning (0-3 per doc)
    int docStudyPasses[MAX_DOCUMENTATION];

    // AI skill knowledge checkpoints (0, 25, 50, 75 per skill)
    int skillCheckpoints[NUM_SKILLS];

    // Intel underground market
    int intelInventory[(int)IntelItemType::COUNT];
    int intelMarketStock[(int)IntelItemType::COUNT];
    float intelPriceMultipliers[(int)IntelItemType::COUNT];
    int intelMarketRefreshTimer;
    int intelScrollOffset;

    // Game hack ban cooldown (seconds remaining)
    int gameHackBanTimer;

    // Heat compounding - feds build a case without protective gear
    int consecutiveJobsNoProtection;
    float heatCompoundMultiplier;

    // Job unlock notification queue (for IRC popup)
    int pendingJobUnlockId;  // -1 = none, otherwise job ID to show dialogue for
    bool jobUnlockFromIRC;   // true = unlocked via IRC/BBS job

    // Screen melt trigger (when personnel hacks you)
    bool triggerScreenMelt;
    wchar_t meltAttackerName[32];

    // Scam hire counter - get scammed too many times and they hack you
    int scamHireCount;

    // Fed seizure / jail system
    bool jailed;                // currently seized by feds
    int64_t jailEndTimestamp;   // Unix timestamp when jail ends (real wall-clock time)
    bool triggerJail;           // flag to trigger jail effect from GameTick

    // Military cyber route
    MilitaryPhase militaryPhase;
    int64_t militaryEnlistTimestamp;    // Unix timestamp when enlisted
    int militaryDaysCompleted;          // 0-4
    int64_t militaryDayStartTimestamp;  // Start of current day (Unix timestamp)
    SkillId militarySpecialty;          // Chosen specialty skill
    int militaryPayTimer;               // Ticks since last paycheck
    int64_t militaryTotalPay;           // Total earned during service
    bool militaryEnlistAvailable;       // Shows enlist option after T4 boss
    bool cyberVeteran;                  // Permanent perk after service
    int militaryBusyworkTimer;          // Ticks since last busywork event
    int militaryScrollOffset;           // UI scroll for military tab
};

// ============================================================================
// Function declarations
// ============================================================================

// game.cpp
void InitGameState(GameState* gs);
void GameTick(GameState* gs);
void AddEventLog(GameState* gs, const wchar_t* text, COLORREF color);
void AddToast(GameState* gs, const wchar_t* text, COLORREF color);
PlayerTitle GetPlayerTitle(int reputation);
int GetMaxJobSlots(const GameState* gs);
int GetMaxPersonnelSlots(const GameState* gs);
int GetActiveJobCount(const GameState* gs);
float GetSkillSpeedBonus(const GameState* gs, int jobDefId);
float GetSkillPayoutBonus(const GameState* gs, int jobDefId);
bool CanStartJob(const GameState* gs, int jobDefId);
bool StartJob(GameState* gs, int jobDefId, int personnelId = -1);
int FindIdlePersonnelForJob(const GameState* gs, int jobDefId); // -1 if none idle; prefers specialty match
uint32_t GameRandom(GameState* gs);
int GameRandomRange(GameState* gs, int minVal, int maxVal);
bool ArePrestigeRequirementsMet(const GameState* gs);
int64_t GetNextPersonnelSlotCost(const GameState* gs);
int GetBossIndexForTier(Tier tier);
int GetBossRequiredRep(int bossIdx); // reputation needed before a tier-gate boss appears
bool CanAttemptBoss(const GameState* gs, int bossIdx);

// Multitasking penalties
struct MultitaskPenalty { float timeMult; float xpMult; };
MultitaskPenalty GetMultitaskPenalty(const GameState* gs);
int GetNonExemptJobCount(const GameState* gs); // excludes BBS/IRC

// Intel market
float GetIntelTimeReduction(const GameState* gs, int jobDefId);
void RefreshIntelMarketStock(GameState* gs);

// Military cyber route
void EnlistMilitary(GameState* gs);
void ProcessMilitaryTick(GameState* gs);
void SelectMilitarySpecialty(GameState* gs, SkillId specialty);
void CompleteMilitaryService(GameState* gs);
bool IsMilitaryActive(const GameState* gs);

// Offline progress (replaces full GameTick simulation)
void ProcessOfflineProgress(GameState* gs, int64_t elapsed);

// jobs.cpp
int GetJobCount();
const JobDef& GetJobDef(int index);
void InitJobCatalog();
int GetDocCount();
const DocStudyDef& GetDocDef(int index);
const BossHackDef& GetBossHackDef(int index);

// save.cpp
bool SaveGame(const GameState* gs);
bool LoadGame(GameState* gs);
bool DeleteSaveFile();

// ============================================================================
// Shared data (defined in game.cpp / ui.cpp, declared here for cross-TU access)
// ============================================================================

// game.cpp
extern const PersonnelTypeDef g_PersonnelTypes[(int)PersonnelType::COUNT];
extern const wchar_t* g_HandlePrefixes[];
extern const wchar_t* g_HandleSuffixes[];
extern const int NUM_PREFIXES;
extern const int NUM_SUFFIXES;

// Special job IDs
constexpr int JOB_ID_BBS_MODERATION = 5;
constexpr int JOB_ID_IRC_ADMIN = 6;
constexpr int JOB_ID_WEBSITE_DEFACEMENT = 0;
constexpr int JOB_ID_DDOS_MINECRAFT = 13;
constexpr int JOB_ID_DARKNET_MARKET_ADMIN = 43;
constexpr int JOB_ID_FORUM_TIP_FOLLOWUP = 86;
constexpr int JOB_ID_GAME_HACK_FIRST = 87;
constexpr int JOB_ID_GAME_HACK_LAST = 94;
constexpr int JOB_ID_CONTRACTOR_FIRST = 95;
constexpr int JOB_ID_CONTRACTOR_LAST = 102;

// Check if a job is a "passive/admin" job that doesn't cause multitask penalties
inline bool IsPassiveJob(int jobDefId)
{
    return JOB_ID_BBS_MODERATION == jobDefId ||
           JOB_ID_IRC_ADMIN == jobDefId ||
           JOB_ID_DARKNET_MARKET_ADMIN == jobDefId;
}

// Check if a job is a game hacking job
inline bool IsGameHackJob(int jobDefId)
{
    return jobDefId >= JOB_ID_GAME_HACK_FIRST && jobDefId <= JOB_ID_GAME_HACK_LAST;
}

// Check if a job is a contractor/mercenary job (military veteran route)
inline bool IsContractorJob(int jobDefId)
{
    return jobDefId >= JOB_ID_CONTRACTOR_FIRST && jobDefId <= JOB_ID_CONTRACTOR_LAST;
}

// Hacker usernames and intel items are declared in src/data/strings.h

// ui.cpp
extern const MarketItem g_MarketItems[];
extern const int g_NumMarketItems;
