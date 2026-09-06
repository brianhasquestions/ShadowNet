#include "hacker_stage.h"
#include "colors.h"
#include <strsafe.h>
#include <iterator>
#include <memory>

// ============================================================================
// Grid and Layout Constants
// ============================================================================

constexpr int kGridW = 48;                 // scene width in sprite pixels
constexpr int kGridH = 26;                 // scene height in sprite pixels
constexpr int kStripScale = 4;             // sprite pixel size in the strip
constexpr int kCompactScale = 2;           // sprite pixel size in the overlay
constexpr int kStripSceneX = 16;
constexpr int kStripSceneY = 8;
constexpr int kCompactSceneX = 6;
constexpr int kCompactSceneY = 8;
constexpr int kEdgeLineHeight = 2;
constexpr int kGradientBands = 8;
constexpr int kCeilingTileW = 12;          // grid units
constexpr int kCeilingTileH = 6;

constexpr int kDeskY = 22;
constexpr int kDeskX = 12;
constexpr int kFloorY = 24;
constexpr int kWallH = 22;
constexpr int kWindowX = 1;
constexpr int kWindowY = 1;
constexpr int kWindowW = 33;
constexpr int kWindowH = 13;
constexpr int kWindowBarY = 7;
constexpr int kScreenX = 35;
constexpr int kScreenY = 9;
constexpr int kScreenCols = 12;
constexpr int kScreenRows = 9;
constexpr int kScreenBustedRow = 4;
constexpr int kTubeX = 8;
constexpr int kTubeW = 30;
constexpr int kPartitionW = 34;
constexpr int kPartitionY = 3;
constexpr int kMemoPinX = 25;
constexpr int kMemoPinY = 5;
constexpr int kCalendarX = 29;
constexpr int kCalendarY = 5;
constexpr int kCalendarW = 3;
constexpr int kCalendarH = 4;
constexpr int kSteamAX = 37;
constexpr int kSteamAY = 17;
constexpr int kSteamBX = 38;
constexpr int kSteamBY = 16;
constexpr int kNearHandX = 26;
constexpr int kFarHandX = 30;
constexpr int kHandRestY = 19;
constexpr int kHandUpY = 18;
constexpr int kHandOffKeysY = 17;
constexpr int kRaisedHandX = 25;
constexpr int kRaisedHandY = 7;
constexpr int kBackArmOffsetX = -3;
constexpr int kBackHandX = 22;
constexpr int kBackHandY = 7;
constexpr int kBangX = 14;
constexpr int kBangY = 0;
constexpr int kSweatSpawnX = 20;
constexpr int kSweatSpawnY = 6;
constexpr int kDollarSpawnX = 26;
constexpr int kDollarSpawnRangeX = 8;
constexpr int kDollarSpawnY = 17;
constexpr int kAlarmRecoilX = -2;
constexpr int kAlarmRecoilY = -1;
constexpr int kBustedHeadY = -1;

// Readout box (strip)
constexpr int kReadoutGap = 28;
constexpr int kReadoutMargin = 10;
constexpr int kReadoutRight = 16;
constexpr int kReadoutTextX = 16;
constexpr int kReadoutCaptionY = 8;
constexpr int kReadoutMutterY = 44;
constexpr int kReadoutLastY = 70;

// Compact caption (overlay)
constexpr int kCompactTextX = 108;
constexpr int kCompactCaptionY = 12;
constexpr int kCompactMutterY = 34;
constexpr int kCompactTextRight = 4;

// ============================================================================
// Timing Constants (milliseconds unless noted)
// ============================================================================

constexpr int kAlarmRedMs = 2500;
constexpr int kAlarmOrangeMs = 1500;
constexpr int kCelebrateMs = 2000;
constexpr int kNoticeMs = 1500;
constexpr int kBlinkMs = 150;
constexpr int kBlinkMinGapMs = 2500;
constexpr int kBlinkRandGapMs = 3000;
constexpr int kMutterMs = 5000;
constexpr int kDollarSpawnMs = 300;
constexpr int kDollarLifeMs = 1400;
constexpr int kSweatLifeMs = 900;
constexpr int kSweatGapMs = 1100;
constexpr int kSweatGapAlarmMs = 500;
constexpr int kScreenRateTypingMs = 150;
constexpr int kScreenRateIdleMs = 900;
constexpr int kScreenRateOtherMs = 250;
constexpr int kIdleBobMs = 400;
constexpr int kTypingBobMs = 250;
constexpr int kTypingBobFastMs = 150;
constexpr int kCelebrateBobMs = 150;
constexpr int kAlarmShakeMs = 100;
constexpr int kBustedBobMs = 600;
constexpr int kHandsMs = 200;
constexpr int kHandsFastMs = 120;
constexpr int kCursorMs = 500;
constexpr int kSteamMs = 500;
constexpr int kSirenMs = 300;
constexpr int kAntennaMs = 700;
constexpr int kWindowFlickerMs = 1800;
constexpr int kTubeFlickerPeriodMs = 4000;
constexpr int kTubeFlickerMs = 90;
constexpr int kTubeFlickerSlot = 7;       // flicker only in the last of 8 slots
constexpr int kTubeFlickerSlots = 8;
constexpr int kTwoPhase = 2;              // on / off sequences
constexpr int kWindowSpacing = 2;         // lit windows every other pixel
constexpr int kFlickerSeedModulo = 1000;
constexpr int kFlickerSeedScaleMs = 10;
constexpr int kSweatDropH = 2;
constexpr int kBustedBlinkMs = 400;

// Thresholds
constexpr int kSweatHeat = 60;
constexpr int kSirenHeat = 85;
constexpr int kMultiJobThreshold = 3;
constexpr int kMilitaryTotalDays = 4;

// Particles and rain
constexpr int kMaxParticles = 16;
constexpr int kRainColumns = 56;
constexpr float kRainMinSpeed = 4.0f;       // grid rows per second
constexpr float kRainRandSpeed = 10.0f;
constexpr int kRainMinLen = 3;
constexpr int kRainRandLen = 5;
constexpr float kRainWrapY = 40.0f;
constexpr float kRainRespawnRange = 20.0f;
constexpr float kRainPinkChance = 0.25f;
constexpr float kDollarMinSpeed = -6.0f;    // grid rows per second (upward)
constexpr float kDollarRandSpeed = -4.0f;
constexpr float kSweatSpeedY = 5.0f;
constexpr float kSweatSpeedX = 1.5f;
constexpr float kMsPerSecond = 1000.0f;

constexpr uint32_t kRngSeed = 0x9E3779B9u;
constexpr int kCodeColorCount = 5;
constexpr int kCodeMaxIndent = 2;
constexpr int kCodeMinLen = 2;
constexpr int kCodeRandLen = 7;

// ============================================================================
// Palettes
// ============================================================================

struct ScenePalette
{
    // Sprite keys
    COLORREF hood;          // 'H'  hood (uniform olive in the cubicle)
    COLORREF hoodFold;      // 'h'
    COLORREF rimBack;       // 'P'  rim light on the back edge
    COLORREF rimFront;      // 'B'  rim light on the screen side
    COLORREF faceVoid;      // 'F'
    COLORREF eye;           // 'E'  glowing eyes under the hood
    COLORREF skin;          // 'S'
    COLORREF skinShade;     // 's'
    COLORREF hair;          // 'A'
    COLORREF eyeDark;       // 'X'  eye on the uniformed head
    COLORREF uniform;       // 'U'
    COLORREF uniformLight;  // 'u'
    COLORREF nameTape;      // 'T'
    COLORREF chair;         // 'C'
    COLORREF chairLite;     // 'c'
    COLORREF keyBase;       // 'K'
    COLORREF keyCap;        // 'k'
    COLORREF monitor;       // 'M'
    COLORREF monitorLite;   // 'n'
    COLORREF screen;        // 'm'
    COLORREF led;           // 'L'
    COLORREF white;         // 'W'
    COLORREF paper;         // 'p'
    COLORREF desk;          // 'D'
    COLORREF deskEdge;      // 'd'
    COLORREF accent;        // 'Q'  mug handle, notice glyph
    // Room and strip
    COLORREF bgTop;
    COLORREF bgBottom;
    COLORREF wall;
    COLORREF floor;
    COLORREF edge;
    COLORREF codeColors[kCodeColorCount];
    COLORREF caption;
    COLORREF mutter;
    COLORREF dim;
    COLORREF payday;
    COLORREF paydayAlt;
    COLORREF steam;
};

constexpr COLORREF kNeonPink = RGB(255, 64, 208);
constexpr COLORREF kNeonPinkDim = RGB(120, 24, 106);
constexpr COLORREF kNeonBlue = RGB(64, 208, 255);
constexpr COLORREF kNeonBlueDim = RGB(28, 90, 140);
constexpr COLORREF kNeonBlueDark = RGB(16, 40, 72);
constexpr COLORREF kNeonPurple = RGB(176, 112, 208);
constexpr COLORREF kAlarmRed = RGB(255, 74, 90);
constexpr COLORREF kAlarmRedDim = RGB(128, 32, 48);
constexpr COLORREF kAlarmScreenOn = RGB(90, 16, 32);
constexpr COLORREF kAlarmScreenOff = RGB(42, 8, 16);
constexpr COLORREF kBustedScreen = RGB(6, 10, 20);
constexpr COLORREF kSkyTop = RGB(22, 48, 90);
constexpr COLORREF kSkyBottom = RGB(90, 26, 104);
constexpr COLORREF kSkylineInk = RGB(14, 8, 32);
constexpr COLORREF kWindowFrame = RGB(58, 42, 90);
constexpr COLORREF kReadoutFill = RGB(12, 8, 28);
constexpr COLORREF kTube = RGB(238, 244, 224);
constexpr COLORREF kTubeGlow = RGB(92, 102, 80);
constexpr COLORREF kPartition = RGB(86, 90, 78);
constexpr COLORREF kPartitionRail = RGB(140, 144, 126);
constexpr COLORREF kMemoPaper = RGB(216, 212, 196);
constexpr COLORREF kMemoInk = RGB(90, 88, 80);
constexpr COLORREF kMemoPin = RGB(192, 64, 64);
constexpr COLORREF kCalendarHeader = RGB(138, 32, 32);
constexpr COLORREF kCeilingLine = RGB(36, 38, 32);
constexpr COLORREF kPhosphor = RGB(72, 224, 112);
constexpr COLORREF kPhosphorDim = RGB(28, 106, 52);
constexpr COLORREF kPhosphorLite = RGB(140, 240, 168);
constexpr COLORREF kKhaki = RGB(200, 184, 140);
constexpr float kSirenTint = 0.12f;
constexpr float kHorizonTint = 0.18f;
constexpr int kHorizonGlowH = 3;

static const ScenePalette kApartment =
{
    RGB(26, 24, 48),     // hood
    RGB(48, 44, 76),     // hoodFold
    RGB(208, 64, 176),   // rimBack
    RGB(72, 184, 240),   // rimFront
    RGB(4, 3, 10),       // faceVoid
    RGB(96, 248, 255),   // eye
    RGB(236, 200, 182),  // skin
    RGB(212, 168, 144),  // skinShade
    RGB(42, 32, 24),     // hair
    RGB(26, 20, 16),     // eyeDark
    RGB(76, 90, 52),     // uniform
    RGB(106, 122, 72),   // uniformLight
    kKhaki,              // nameTape
    RGB(42, 34, 70),     // chair
    RGB(64, 56, 102),    // chairLite
    RGB(40, 40, 64),     // keyBase
    RGB(84, 84, 120),    // keyCap
    RGB(46, 46, 70),     // monitor
    RGB(70, 70, 100),    // monitorLite
    RGB(12, 24, 44),     // screen
    kNeonBlue,           // led
    RGB(240, 240, 255),  // white
    RGB(230, 230, 240),  // paper
    RGB(44, 30, 72),     // desk
    RGB(120, 70, 190),   // deskEdge
    kNeonPink,           // accent
    RGB(10, 7, 26),      // bgTop
    RGB(24, 12, 44),     // bgBottom
    RGB(26, 16, 52),     // wall
    RGB(24, 12, 44),     // floor
    kNeonBlueDim,        // edge
    { kNeonBlue, kNeonPink, kNeonBlueDim, RGB(192, 192, 224), kNeonBlueDim },
    kNeonPink,           // caption
    kNeonBlue,           // mutter
    RGB(90, 84, 128),    // dim
    Colors::Green,       // payday
    kNeonPink,           // paydayAlt
    RGB(138, 138, 168),  // steam
};

static const ScenePalette kCubicle =
{
    RGB(76, 90, 52),     // hood -> uniform sleeve
    RGB(62, 74, 44),     // hoodFold
    RGB(62, 74, 44),     // rimBack
    RGB(106, 122, 72),   // rimFront
    RGB(4, 3, 10),       // faceVoid
    RGB(96, 248, 255),   // eye
    RGB(236, 200, 182),  // skin
    RGB(212, 168, 144),  // skinShade
    RGB(42, 32, 24),     // hair
    RGB(26, 20, 16),     // eyeDark
    RGB(76, 90, 52),     // uniform
    RGB(106, 122, 72),   // uniformLight
    kKhaki,              // nameTape
    RGB(62, 62, 60),     // chair
    RGB(88, 88, 86),     // chairLite
    RGB(58, 58, 54),     // keyBase
    RGB(110, 110, 104),  // keyCap
    RGB(184, 176, 156),  // monitor
    RGB(212, 204, 180),  // monitorLite
    RGB(10, 36, 20),     // screen
    kPhosphor,           // led
    RGB(240, 240, 255),  // white
    RGB(216, 212, 196),  // paper
    RGB(110, 110, 106),  // desk
    RGB(154, 154, 148),  // deskEdge
    RGB(106, 122, 72),   // accent
    RGB(42, 44, 40),     // bgTop
    RGB(30, 32, 28),     // bgBottom
    RGB(58, 60, 54),     // wall
    RGB(44, 46, 42),     // floor
    RGB(122, 138, 106),  // edge
    { kPhosphor, kPhosphorDim, kPhosphor, kPhosphorLite, kPhosphorDim },
    kKhaki,              // caption
    kPhosphor,           // mutter
    RGB(122, 122, 106),  // dim
    kPhosphor,           // payday
    kPhosphorLite,       // paydayAlt
    RGB(138, 138, 130),  // steam
};

// ============================================================================
// Sprites
//
// One character per sprite pixel, '.' is transparent. The letters map to
// ScenePalette fields (see PaletteColor). The hacker faces right.
// ============================================================================

struct Sprite
{
    int x;
    int y;
    int rows;
    const char* const* map;
};

template <size_t N>
static constexpr Sprite MakeSprite(int x, int y, const char* const (&map)[N])
{
    return Sprite{ x, y, static_cast<int>(N), map };
}

static const char* const kHeadHoodMap[] =
{
    "....HHHHHHHH....",
    "..PHHHHHHHHHHH..",
    ".PHHHHHHHHHHHHH.",
    ".PHHhHHHHHHHHHHB",
    ".PHHhHHHHHFFFFHB",
    ".PHHhHHHHFFFFFF.",
    ".PHHhHHHHFFFEEF.",
    ".PHHhHHHHFFFFFF.",
    ".PHHHHHHHHFFFFF.",
    "..PHHHHHHHHFFF..",
    "..HHHHHHHHHHH...",
    "...HHHHHHHHH....",
};
static const Sprite kHeadHood = MakeSprite(6, 4, kHeadHoodMap);

static const char* const kHeadCapMap[] =
{
    "....UUUUUUUU....",
    "...UUUUUUUUUUU..",
    "..UUUUUUUUUUUUU.",
    "..UUUUUUUUUUuuuu",
    "..AASSSSSSSSSS..",
    "..AASsSSSSSSSSS.",
    "..AASsSSSSSSXSS.",
    "...ASSSSSSSSSSS.",
    "....SSSSSSSSSSS.",
    "....SSSSSSSSSS..",
    ".....SSSSSSSS...",
    "......SSSSSS....",
};
static const Sprite kHeadCap = MakeSprite(6, 4, kHeadCapMap);

static const char* const kBodyHoodMap[] =
{
    "....HHHHHHHHHH....",
    "..PHHHHHHHHHHHHHH.",
    ".PHHHHHHHHHHHHHHHB",
    ".PHHhHHHHHHHHHHHHB",
    ".PHHhHHHHHHHHHHHH.",
    ".PHHhHHHHHHHHHHHH.",
    ".PHHHHHHHHHHHHHHH.",
    ".PHHHHHHHHHHHHHHH.",
};
static const Sprite kBodyHood = MakeSprite(4, 15, kBodyHoodMap);

static const char* const kBodyUniformMap[] =
{
    "....UUUuuuuUUU....",
    "..hUUUUUuuUUUUUUu.",
    ".hUUUUUUUTTTTUUUUu",
    ".hUUUUUUUUUUUUUUUu",
    ".hUUUUUUUUUUUUUUU.",
    ".hUUUUUUUUUUUUUUU.",
    ".hUUUUUUUUUUUUUUU.",
    ".hUUUUUUUUUUUUUUU.",
};
static const Sprite kBodyUniform = MakeSprite(4, 15, kBodyUniformMap);

static const char* const kChairMap[] =
{
    ".CCC.",
    "CcCCC",
    "CcCCC",
    "CcCCC",
    "CcCCC",
    "CcCCC",
    "CcCCC",
    "CcCCC",
    "CcCCC",
    "CcCCC",
    "CCCCC",
    "CCCCC",
    "CCCCC",
    "CCCCC",
    ".CCC.",
    "..C..",
    "..C..",
};
static const Sprite kChair = MakeSprite(1, 9, kChairMap);

static const char* const kSleeveTypingMap[] =
{
    "HHHH........",
    ".HHHHHHH....",
    "...HHHHHHH..",
    "......HHHHH.",
};
static const Sprite kSleeveTyping = MakeSprite(15, 16, kSleeveTypingMap);

static const char* const kArmRaisedMap[] =
{
    "..........",
    ".......HH.",
    ".......HH.",
    ".......HH.",
    ".......HH.",
    "......HHH.",
    "...HHHHH..",
    "HHHHH.....",
    "HH........",
};
static const Sprite kArmRaised = MakeSprite(18, 8, kArmRaisedMap);

static const char* const kKeyboardMap[] =
{
    "kKkKkKkKkK",
    "KKKKKKKKKK",
};
static const Sprite kKeyboard = MakeSprite(24, 20, kKeyboardMap);

static const char* const kMonitorMap[] =
{
    "nMMMMMMMMMMMMM",
    "MmmmmmmmmmmmmM",
    "MmmmmmmmmmmmmM",
    "MmmmmmmmmmmmmM",
    "MmmmmmmmmmmmmM",
    "MmmmmmmmmmmmmM",
    "MmmmmmmmmmmmmM",
    "MmmmmmmmmmmmmM",
    "MmmmmmmmmmmmmM",
    "MmmmmmmmmmmmmM",
    "MMMMMMMMMMMLMM",
    "......MM......",
    "......MM......",
    "...MMMMMMMM...",
};
static const Sprite kMonitor = MakeSprite(34, 8, kMonitorMap);

static const char* const kMugMap[] =
{
    "ppQ",
    "ppQ",
    "pp.",
};
static const Sprite kMug = MakeSprite(36, 19, kMugMap);

static const char* const kPaperMap[] =
{
    "pppppp",
    "pBBBpp",
    "pppppp",
    "pBBBBp",
    "pppppp",
    "pBBppp",
    "pppppp",
};
static const Sprite kPaper = MakeSprite(24, 12, kPaperMap);

static const char* const kMemoMap[] =
{
    "pppp",
    "pXXp",
    "pppp",
    "pXXp",
    "pppp",
};
static const Sprite kMemo = MakeSprite(24, 5, kMemoMap);

static const char* const kDollarGlyph[] = { ".WW", "W..", ".W.", "..W", "WW." };
static const char* const kBangGlyph[] = { "Q", "Q", "Q", ".", "Q" };

// City skyline inside the apartment window. x and w in window columns,
// h counted up from the sill.
struct Building
{
    int x;
    int w;
    int h;
    bool antenna;
};

static const Building kSkyline[] =
{
    { 0, 3, 6, false }, { 3, 2, 8, false }, { 6, 4, 5, false }, { 10, 3, 10, true },
    { 13, 2, 4, false }, { 16, 4, 7, false }, { 20, 3, 5, false }, { 23, 2, 9, true },
    { 25, 4, 6, false }, { 29, 3, 8, false }, { 32, 1, 5, false },
};

// ============================================================================
// Captions and Mutters
// ============================================================================

enum class StageMood : int
{
    Idle = 0,
    Typing,
    Reading,
    Celebrate,
    Alarm,
    Busted,
    COUNT
};

struct MutterPool
{
    const wchar_t* const* lines;
    int count;
};

template <size_t N>
static constexpr MutterPool MakePool(const wchar_t* const (&lines)[N])
{
    return MutterPool{ lines, static_cast<int>(N) };
}

static const wchar_t* const kMutterIdle[] = { L"refreshing the forums...", L"stares at the cursor.", L"hums dial-up tones.", L"checks the VPN. again.", L"reads a 1998 zine." };
static const wchar_t* const kMutterTyping[] = { L"tap tap tap tap tap", L"just one more compile.", L"route it through three proxies. four.", L"who wrote this stack?", L"zero-day o'clock." };
static const wchar_t* const kMutterReading[] = { L"RTFM, apparently.", L"page 47 of 900.", L"the highlighter is out of ink." };
static const wchar_t* const kMutterCelebrate[] = { L"ka-ching.", L"gg ez.", L"another one for the pile." };
static const wchar_t* const kMutterAlarm[] = { L"who is knocking?", L"wipe the logs. WIPE THEM.", L"it wasn't me." };
static const wchar_t* const kMutterBusted[] = { L"lawyer up.", L"should have bought the VPN.", L"counting ceiling tiles." };

static const wchar_t* const kMutterMilIdle[] = { L"hurry up and wait.", L"the coffee is regulation.", L"1400 hours. still in a meeting." };
static const wchar_t* const kMutterMilTyping[] = { L"PowerPoint, sir.", L"ticket closed. ticket reopened.", L"this keyboard is older than me.", L"yes sir. no sir. rebooting, sir." };
static const wchar_t* const kMutterMilReading[] = { L"field manual, chapter 9.", L"mandatory training module 14." };
static const wchar_t* const kMutterMilCelebrate[] = { L"payday. direct deposit.", L"promotion points!" };
static const wchar_t* const kMutterMilAlarm[] = { L"inspection. INSPECTION.", L"who touched the classified box?" };
static const wchar_t* const kMutterMilBusted[] = { L"contractor burn.", L"clearance revoked.", L"so much for the pension." };

static const MutterPool kMutters[static_cast<int>(StageMood::COUNT)] =
{
    MakePool(kMutterIdle), MakePool(kMutterTyping), MakePool(kMutterReading),
    MakePool(kMutterCelebrate), MakePool(kMutterAlarm), MakePool(kMutterBusted),
};

static const MutterPool kMilitaryMutters[static_cast<int>(StageMood::COUNT)] =
{
    MakePool(kMutterMilIdle), MakePool(kMutterMilTyping), MakePool(kMutterMilReading),
    MakePool(kMutterMilCelebrate), MakePool(kMutterMilAlarm), MakePool(kMutterMilBusted),
};

// ============================================================================
// Animation State
// ============================================================================

struct CodeLine
{
    int indent;
    int len;
    int colorIdx;
};

struct RainDrop
{
    float xFrac;    // 0..1 across the strip width
    float y;        // grid rows
    float speed;    // grid rows per second
    int len;
    bool pink;
};

struct Particle
{
    bool active;
    bool dollar;    // true = rising $, false = sweat drop
    float x;
    float y;
    float vx;
    float vy;
    int lifeMs;
    COLORREF color;
};

struct StageState
{
    bool initialized;
    uint32_t rng;
    int timeMs;
    StageMood mood;
    int moodMs;             // time spent in the current mood
    StageMood overrideMood; // Celebrate or Alarm while overrideMs > 0
    int overrideMs;
    bool military;
    int activeJobs;
    int heat;
    int noticeMs;
    int blinkMs;
    int blinkGapMs;
    int screenMs;
    int mutterIndex;
    int mutterMs;
    int sweatMs;
    int dollarMs;
    CodeLine screen[kScreenRows];
    RainDrop rain[kRainColumns];
    Particle particles[kMaxParticles];
    // Snapshot of GameState for change detection
    int prevEventLogHead;
    int prevJobsCompleted;
};

static StageState g_Stage = {};

// ============================================================================
// Random Helpers (private generator, never touches GameState's rng)
// ============================================================================

static uint32_t StageRandom()
{
    uint32_t x = g_Stage.rng;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    g_Stage.rng = x;
    return x;
}

static int StageRandomRange(int minVal, int maxVal)
{
    return minVal + static_cast<int>(StageRandom() % static_cast<uint32_t>(maxVal - minVal + 1));
}

static float StageRandomUnit()
{
    constexpr uint32_t kMask = 0xFFFF;
    return static_cast<float>(StageRandom() & kMask) / static_cast<float>(kMask);
}

// Index into a repeating sequence of `count` steps, each `periodMs` long.
static int Step(int periodMs, int count)
{
    return (g_Stage.timeMs / periodMs) % count;
}

// Deterministic per-pixel hash for lit windows in the skyline.
static uint32_t PixelHash(int x, int y)
{
    constexpr uint32_t kMulX = 73856093u;
    constexpr uint32_t kMulY = 19349663u;
    constexpr uint32_t kMix = 1274126177u;
    constexpr int kShift = 13;
    uint32_t h = (static_cast<uint32_t>(x) * kMulX) ^ (static_cast<uint32_t>(y) * kMulY);
    h = (h ^ (h >> kShift)) * kMix;
    return h;
}

// ============================================================================
// State Update
// ============================================================================

static CodeLine NewCodeLine()
{
    CodeLine line;
    line.indent = StageRandomRange(0, kCodeMaxIndent);
    line.len = StageRandomRange(kCodeMinLen, kCodeMinLen + kCodeRandLen);
    line.colorIdx = StageRandomRange(0, kCodeColorCount - 1);
    return line;
}

static void InitStage(const GameState* gs)
{
    StageState& s = g_Stage;
    s.rng = kRngSeed ^ static_cast<uint32_t>(GetTickCount());
    if (0 == s.rng)
    {
        s.rng = kRngSeed;
    }

    for (int i = 0; i < kRainColumns; i++)
    {
        s.rain[i].xFrac = StageRandomUnit();
        s.rain[i].y = StageRandomUnit() * kRainWrapY;
        s.rain[i].speed = kRainMinSpeed + StageRandomUnit() * kRainRandSpeed;
        s.rain[i].len = StageRandomRange(kRainMinLen, kRainMinLen + kRainRandLen);
        s.rain[i].pink = StageRandomUnit() < kRainPinkChance;
    }
    for (int r = 0; r < kScreenRows; r++)
    {
        s.screen[r] = NewCodeLine();
    }
    s.blinkGapMs = kBlinkMinGapMs + StageRandomRange(0, kBlinkRandGapMs);
    s.mood = StageMood::Idle;
    s.overrideMood = StageMood::Idle;

    // Snapshot without reacting, so loading a save does not replay old events.
    s.prevEventLogHead = gs->eventLogHead;
    s.prevJobsCompleted = gs->totalJobsCompleted;
    s.initialized = true;
}

static void TriggerOverride(StageMood mood, int durationMs)
{
    StageState& s = g_Stage;
    // An active alarm is not interrupted by a payday.
    if (StageMood::Alarm == s.overrideMood && s.overrideMs > 0 && StageMood::Celebrate == mood)
    {
        return;
    }
    s.overrideMood = mood;
    if (durationMs > s.overrideMs)
    {
        s.overrideMs = durationMs;
    }
}

static void ReactToEvent(COLORREF color)
{
    if (Colors::Red == color)
    {
        TriggerOverride(StageMood::Alarm, kAlarmRedMs);
    }
    else if (Colors::Orange == color)
    {
        TriggerOverride(StageMood::Alarm, kAlarmOrangeMs);
    }
    else if (Colors::Green == color)
    {
        TriggerOverride(StageMood::Celebrate, kCelebrateMs);
    }
    else if (Colors::Comment != color)
    {
        g_Stage.noticeMs = kNoticeMs;
    }
}

static void DetectGameEvents(const GameState* gs)
{
    StageState& s = g_Stage;

    int newEntries = (gs->eventLogHead - s.prevEventLogHead + MAX_EVENT_LOG) % MAX_EVENT_LOG;
    for (int i = 0; i < newEntries; i++)
    {
        const EventLogEntry& entry = gs->eventLog[(s.prevEventLogHead + i) % MAX_EVENT_LOG];
        ReactToEvent(entry.color);
    }
    s.prevEventLogHead = gs->eventLogHead;

    if (gs->totalJobsCompleted > s.prevJobsCompleted)
    {
        TriggerOverride(StageMood::Celebrate, kCelebrateMs);
    }
    s.prevJobsCompleted = gs->totalJobsCompleted;
}

static void ResolveMood(const GameState* gs, int dtMs)
{
    StageState& s = g_Stage;

    if (s.overrideMs > 0)
    {
        s.overrideMs -= dtMs;
    }
    if (s.noticeMs > 0)
    {
        s.noticeMs -= dtMs;
    }

    StageMood next = StageMood::Idle;
    if (true == gs->jailed)
    {
        next = StageMood::Busted;
    }
    else if (s.overrideMs > 0)
    {
        next = s.overrideMood;
    }
    else if (true == s.military || s.activeJobs > 0)
    {
        next = StageMood::Typing;
    }
    else if (true == gs->docStudyActive)
    {
        next = StageMood::Reading;
    }

    if (next != s.mood)
    {
        s.mood = next;
        s.moodMs = 0;
        s.dollarMs = 0;
    }
    else
    {
        s.moodMs += dtMs;
    }
}

static void SpawnParticle(const Particle& p)
{
    for (int i = 0; i < kMaxParticles; i++)
    {
        if (false == g_Stage.particles[i].active)
        {
            g_Stage.particles[i] = p;
            g_Stage.particles[i].active = true;
            return;
        }
    }
}

static void UpdateParticles(int dtMs)
{
    StageState& s = g_Stage;
    const ScenePalette& pal = s.military ? kCubicle : kApartment;
    float dt = static_cast<float>(dtMs) / kMsPerSecond;

    if (StageMood::Celebrate == s.mood)
    {
        s.dollarMs += dtMs;
        if (s.dollarMs >= kDollarSpawnMs)
        {
            s.dollarMs = 0;
            Particle p = {};
            p.dollar = true;
            p.x = static_cast<float>(kDollarSpawnX) + StageRandomUnit() * static_cast<float>(kDollarSpawnRangeX);
            p.y = static_cast<float>(kDollarSpawnY);
            p.vy = kDollarMinSpeed + StageRandomUnit() * kDollarRandSpeed;
            p.lifeMs = kDollarLifeMs;
            p.color = (0 == StageRandomRange(0, kTwoPhase - 1)) ? pal.payday : pal.paydayAlt;
            SpawnParticle(p);
        }
    }

    bool sweaty = s.heat >= kSweatHeat || StageMood::Alarm == s.mood;
    s.sweatMs += dtMs;
    int sweatGap = (StageMood::Alarm == s.mood) ? kSweatGapAlarmMs : kSweatGapMs;
    if (true == sweaty && s.sweatMs >= sweatGap)
    {
        s.sweatMs = 0;
        Particle p = {};
        p.dollar = false;
        p.x = static_cast<float>(kSweatSpawnX);
        p.y = static_cast<float>(kSweatSpawnY);
        p.vx = kSweatSpeedX;
        p.vy = kSweatSpeedY;
        p.lifeMs = kSweatLifeMs;
        p.color = kNeonBlue;
        SpawnParticle(p);
    }

    for (int i = 0; i < kMaxParticles; i++)
    {
        Particle& p = s.particles[i];
        if (false == p.active)
        {
            continue;
        }
        p.lifeMs -= dtMs;
        p.x += p.vx * dt;
        p.y += p.vy * dt;
        if (p.lifeMs <= 0)
        {
            p.active = false;
        }
    }
}

static void UpdateAmbient(int dtMs)
{
    StageState& s = g_Stage;
    float dt = static_cast<float>(dtMs) / kMsPerSecond;

    for (int i = 0; i < kRainColumns; i++)
    {
        RainDrop& d = s.rain[i];
        d.y += d.speed * dt;
        if (d.y > kRainWrapY)
        {
            d.y = -static_cast<float>(d.len) - StageRandomUnit() * kRainRespawnRange;
            d.xFrac = StageRandomUnit();
        }
    }

    int screenRate = kScreenRateOtherMs;
    if (StageMood::Typing == s.mood)
    {
        screenRate = kScreenRateTypingMs;
    }
    else if (StageMood::Idle == s.mood || StageMood::Reading == s.mood)
    {
        screenRate = kScreenRateIdleMs;
    }
    s.screenMs += dtMs;
    if (s.screenMs >= screenRate)
    {
        s.screenMs = 0;
        for (int r = 0; r < kScreenRows - 1; r++)
        {
            s.screen[r] = s.screen[r + 1];
        }
        s.screen[kScreenRows - 1] = NewCodeLine();
    }

    s.blinkGapMs -= dtMs;
    if (s.blinkGapMs <= 0)
    {
        s.blinkMs = kBlinkMs;
        s.blinkGapMs = kBlinkMinGapMs + StageRandomRange(0, kBlinkRandGapMs);
    }
    if (s.blinkMs > 0)
    {
        s.blinkMs -= dtMs;
    }

    s.mutterMs += dtMs;
    if (s.mutterMs >= kMutterMs)
    {
        s.mutterMs = 0;
        s.mutterIndex++;
    }
}

void HackerStageUpdate(const GameState* gs, int dtMs)
{
    StageState& s = g_Stage;
    if (false == s.initialized)
    {
        InitStage(gs);
    }

    s.timeMs += dtMs;
    s.military = IsMilitaryActive(gs);
    s.activeJobs = GetActiveJobCount(gs);
    s.heat = gs->heat;

    DetectGameEvents(gs);
    ResolveMood(gs, dtMs);
    UpdateAmbient(dtMs);
    UpdateParticles(dtMs);
}

// ============================================================================
// Pixel Drawing Helpers
// ============================================================================

// A drawing target: an HDC plus the pixel origin and size of one sprite pixel.
struct StageView
{
    HDC hdc;
    int ox;
    int oy;
    int scale;
};

static COLORREF LerpColor(COLORREF a, COLORREF b, float t)
{
    int r = GetRValue(a) + static_cast<int>((GetRValue(b) - GetRValue(a)) * t);
    int g = GetGValue(a) + static_cast<int>((GetGValue(b) - GetGValue(a)) * t);
    int bl = GetBValue(a) + static_cast<int>((GetBValue(b) - GetBValue(a)) * t);
    return RGB(r, g, bl);
}

static void FillPixelRect(HDC hdc, RECT rc, COLORREF color)
{
    SetDCBrushColor(hdc, color);
    FillRect(hdc, &rc, static_cast<HBRUSH>(GetStockObject(DC_BRUSH)));
}

static void FillGridRect(const StageView& v, int gx, int gy, int gw, int gh, COLORREF color)
{
    RECT rc = { v.ox + gx * v.scale, v.oy + gy * v.scale, v.ox + (gx + gw) * v.scale, v.oy + (gy + gh) * v.scale };
    FillPixelRect(v.hdc, rc, color);
}

static void FillVerticalGradient(HDC hdc, RECT rc, COLORREF top, COLORREF bottom, int bands)
{
    int h = rc.bottom - rc.top;
    for (int i = 0; i < bands; i++)
    {
        int y0 = rc.top + (h * i) / bands;
        int y1 = rc.top + (h * (i + 1)) / bands;
        float t = static_cast<float>(i) / static_cast<float>(bands > 1 ? bands - 1 : 1);
        FillPixelRect(hdc, RECT{ rc.left, y0, rc.right, y1 }, LerpColor(top, bottom, t));
    }
}

static COLORREF PaletteColor(const ScenePalette& pal, char ch, bool eyesClosed)
{
    switch (ch)
    {
        case 'H': return pal.hood;
        case 'h': return pal.hoodFold;
        case 'P': return pal.rimBack;
        case 'B': return pal.rimFront;
        case 'F': return pal.faceVoid;
        case 'E': return eyesClosed ? pal.faceVoid : pal.eye;
        case 'S': return pal.skin;
        case 's': return pal.skinShade;
        case 'A': return pal.hair;
        case 'X': return eyesClosed ? pal.skin : pal.eyeDark;
        case 'U': return pal.uniform;
        case 'u': return pal.uniformLight;
        case 'T': return pal.nameTape;
        case 'C': return pal.chair;
        case 'c': return pal.chairLite;
        case 'K': return pal.keyBase;
        case 'k': return pal.keyCap;
        case 'M': return pal.monitor;
        case 'n': return pal.monitorLite;
        case 'm': return pal.screen;
        case 'L': return pal.led;
        case 'W': return pal.white;
        case 'p': return pal.paper;
        case 'D': return pal.desk;
        case 'd': return pal.deskEdge;
        case 'Q': return pal.accent;
        default:  return pal.white;
    }
}

struct BlitParams
{
    const Sprite* sprite;
    int dx;
    int dy;
    bool eyesClosed;
};

static void BlitSprite(const StageView& v, const ScenePalette& pal, const BlitParams& bp)
{
    for (int y = 0; y < bp.sprite->rows; y++)
    {
        const char* row = bp.sprite->map[y];
        for (int x = 0; '\0' != row[x]; x++)
        {
            if ('.' == row[x])
            {
                continue;
            }
            FillGridRect(v, bp.sprite->x + x + bp.dx, bp.sprite->y + y + bp.dy, 1, 1, PaletteColor(pal, row[x], bp.eyesClosed));
        }
    }
}

static void BlitSpriteAt(const StageView& v, const ScenePalette& pal, const Sprite& sprite)
{
    BlitSprite(v, pal, BlitParams{ &sprite, 0, 0, false });
}

static void BlitGlyph(const StageView& v, const char* const* rows, int rowCount, POINT grid, COLORREF color)
{
    for (int y = 0; y < rowCount; y++)
    {
        for (int x = 0; '\0' != rows[y][x]; x++)
        {
            if ('.' != rows[y][x])
            {
                FillGridRect(v, grid.x + x, grid.y + y, 1, 1, color);
            }
        }
    }
}

static void DrawHand(const StageView& v, const ScenePalette& pal, int gx, int gy)
{
    constexpr int kHandSize = 2;
    FillGridRect(v, gx, gy, kHandSize, kHandSize, pal.skin);
}

// ============================================================================
// Room Backgrounds
// ============================================================================

static void DrawApartmentRoom(const StageView& v)
{
    const ScenePalette& pal = kApartment;
    FillGridRect(v, 0, 0, kGridW, kWallH, pal.wall);

    // Window frame and night sky
    FillGridRect(v, kWindowX - 1, kWindowY - 1, kWindowW + 2, kWindowH + 2, kWindowFrame);
    for (int row = 0; row < kWindowH; row++)
    {
        float t = static_cast<float>(row) / static_cast<float>(kWindowH - 1);
        FillGridRect(v, kWindowX, kWindowY + row, kWindowW, 1, LerpColor(kSkyTop, kSkyBottom, t));
    }

    // Skyline with lit windows
    constexpr int kAntennaBlinkH = 2;
    constexpr uint32_t kDarkEvery = 5;
    constexpr uint32_t kFlickerEvery = 7;
    constexpr uint32_t kHueCount = 3;
    constexpr int kWindowFlickerPhases = kTwoPhase;
    int sill = kWindowY + kWindowH;
    for (const Building& b : kSkyline)
    {
        int bx = kWindowX + b.x;
        int top = sill - b.h;
        FillGridRect(v, bx, top, b.w, b.h, kSkylineInk);
        if (true == b.antenna)
        {
            int ax = bx + b.w / 2;
            FillGridRect(v, ax, top - 1, 1, 1, kSkylineInk);
            COLORREF blink = (0 == Step(kAntennaMs, kTwoPhase)) ? kAlarmRed : kSkylineInk;
            FillGridRect(v, ax, top - kAntennaBlinkH, 1, 1, blink);
        }
        for (int wy = top + 1; wy < sill; wy += kWindowSpacing)
        {
            for (int wx = bx; wx < bx + b.w; wx++)
            {
                if (0 != ((wx - bx) % kWindowSpacing) && b.w > 1)
                {
                    continue;
                }
                uint32_t hv = PixelHash(wx, wy);
                if (0 == hv % kDarkEvery)
                {
                    continue;
                }
                if (0 == hv % kFlickerEvery)
                {
                    int phase = ((g_Stage.timeMs + static_cast<int>(hv % kFlickerSeedModulo) * kFlickerSeedScaleMs) / kWindowFlickerMs) % kWindowFlickerPhases;
                    if (1 == phase)
                    {
                        continue;
                    }
                }
                uint32_t hue = hv % kHueCount;
                COLORREF lit = (0 == hue) ? kNeonPinkDim : ((1 == hue) ? kNeonBlueDim : kNeonPurple);
                FillGridRect(v, wx, wy, 1, 1, lit);
            }
        }
    }

    // Window crossbar
    FillGridRect(v, kWindowX, kWindowBarY, kWindowW, 1, kWindowFrame);
}

static void DrawCubicleRoom(const StageView& v)
{
    const ScenePalette& pal = kCubicle;
    FillGridRect(v, 0, 0, kGridW, kWallH, pal.wall);

    // Fluorescent tube with the occasional flicker
    bool flicker = (kTubeFlickerSlot == Step(kTubeFlickerPeriodMs / kTubeFlickerSlots, kTubeFlickerSlots)) && (0 == Step(kTubeFlickerMs, kTwoPhase));
    FillGridRect(v, kTubeX, 0, kTubeW, 1, flicker ? kTubeGlow : kTube);
    FillGridRect(v, kTubeX, 1, kTubeW, 1, kTubeGlow);

    // Cubicle partition. No window.
    FillGridRect(v, 0, kPartitionY, kPartitionW, kWallH - kPartitionY, kPartition);
    FillGridRect(v, 0, kPartitionY, kPartitionW, 1, kPartitionRail);
    FillGridRect(v, kPartitionW - 1, kPartitionY, 1, kWallH - kPartitionY, kPartitionRail);

    // Pinned memo and a wall calendar
    ScenePalette memoPal = pal;
    memoPal.paper = kMemoPaper;
    memoPal.eyeDark = kMemoInk;
    BlitSpriteAt(v, memoPal, kMemo);
    FillGridRect(v, kMemoPinX, kMemoPinY, 1, 1, kMemoPin);
    FillGridRect(v, kCalendarX, kCalendarY, kCalendarW, kCalendarH, kMemoPaper);
    FillGridRect(v, kCalendarX, kCalendarY, kCalendarW, 1, kCalendarHeader);
}

// ============================================================================
// Monitor Screen Contents
// ============================================================================

static void DrawScreenContents(const StageView& v, const ScenePalette& pal)
{
    const StageState& s = g_Stage;
    FillGridRect(v, kScreenX, kScreenY, kScreenCols, kScreenRows, pal.screen);

    if (StageMood::Alarm == s.mood)
    {
        bool on = (0 == Step(kScreenRateOtherMs, kTwoPhase));
        FillGridRect(v, kScreenX, kScreenY, kScreenCols, kScreenRows, on ? kAlarmScreenOn : kAlarmScreenOff);
        for (int r = 0; r < kScreenRows; r++)
        {
            const CodeLine& line = s.screen[r];
            FillGridRect(v, kScreenX + line.indent, kScreenY + r, line.len, 1, on ? kAlarmRed : kAlarmRedDim);
        }
        return;
    }

    if (StageMood::Busted == s.mood)
    {
        FillGridRect(v, kScreenX, kScreenY, kScreenCols, kScreenRows, kBustedScreen);
        if (0 == Step(kBustedBlinkMs, kTwoPhase))
        {
            FillGridRect(v, kScreenX, kScreenY + kScreenBustedRow, kScreenCols, 1, kAlarmRed);
        }
        return;
    }

    for (int r = 0; r < kScreenRows; r++)
    {
        const CodeLine& line = s.screen[r];
        COLORREF color = pal.codeColors[line.colorIdx];
        if (StageMood::Celebrate == s.mood)
        {
            color = (0 != r % kTwoPhase) ? pal.payday : pal.paydayAlt;
        }
        FillGridRect(v, kScreenX + line.indent, kScreenY + r, line.len, 1, color);
    }
    if (0 == Step(kCursorMs, kTwoPhase))
    {
        const CodeLine& last = s.screen[kScreenRows - 1];
        FillGridRect(v, kScreenX + last.indent + last.len, kScreenY + kScreenRows - 1, 1, 1, pal.white);
    }
}

// ============================================================================
// The Scene
// ============================================================================

struct Pose
{
    int headX;
    int headY;
    bool armsRaised;    // one arm up (payday)
    bool bothArmsUp;    // both arms up (busted)
    bool handsOffKeys;  // alarm recoil
};

static Pose ResolvePose()
{
    const StageState& s = g_Stage;
    Pose pose = {};
    static const int kIdleBob[] = { 0, 0, 1, 1 };
    static const int kTwoStepBob[] = { 0, 1 };
    static const int kCelebrateBob[] = { 0, -1 };
    static const int kShake[] = { 0, -1, 0, 1 };
    static const int kBustedBob[] = { 0, 0, 0, 1 };

    switch (s.mood)
    {
        case StageMood::Idle:
        case StageMood::Reading:
            pose.headY = kIdleBob[Step(kIdleBobMs, static_cast<int>(std::size(kIdleBob)))];
            break;
        case StageMood::Typing:
        {
            int bobMs = (s.activeJobs >= kMultiJobThreshold) ? kTypingBobFastMs : kTypingBobMs;
            pose.headY = kTwoStepBob[Step(bobMs, static_cast<int>(std::size(kTwoStepBob)))];
            break;
        }
        case StageMood::Celebrate:
            pose.headY = kCelebrateBob[Step(kCelebrateBobMs, static_cast<int>(std::size(kCelebrateBob)))];
            pose.armsRaised = true;
            break;
        case StageMood::Alarm:
            pose.headX = kAlarmRecoilX + kShake[Step(kAlarmShakeMs, static_cast<int>(std::size(kShake)))];
            pose.headY = kAlarmRecoilY;
            pose.handsOffKeys = true;
            break;
        case StageMood::Busted:
            pose.headY = kBustedHeadY + kBustedBob[Step(kBustedBobMs, static_cast<int>(std::size(kBustedBob)))];
            pose.bothArmsUp = true;
            break;
        default:
            break;
    }
    return pose;
}

static void DrawArms(const StageView& v, const ScenePalette& pal, const Pose& pose)
{
    const StageState& s = g_Stage;

    if (true == pose.armsRaised || true == pose.bothArmsUp)
    {
        BlitSpriteAt(v, pal, kArmRaised);
        int pump = 0;
        if (StageMood::Celebrate == s.mood)
        {
            static const int kPump[] = { 0, -1 };
            pump = kPump[Step(kCelebrateBobMs, static_cast<int>(std::size(kPump)))];
        }
        DrawHand(v, pal, kRaisedHandX, kRaisedHandY + pump);
        return;
    }

    BlitSpriteAt(v, pal, kSleeveTyping);
    int nearY = kHandRestY;
    int farY = kHandRestY;
    if (StageMood::Typing == s.mood)
    {
        int handsMs = (s.activeJobs >= kMultiJobThreshold) ? kHandsFastMs : kHandsMs;
        bool alt = (1 == Step(handsMs, kTwoPhase));
        nearY = alt ? kHandUpY : kHandRestY;
        farY = alt ? kHandRestY : kHandUpY;
    }
    if (true == pose.handsOffKeys)
    {
        nearY = kHandOffKeysY;
        farY = kHandOffKeysY;
    }
    if (StageMood::Reading == s.mood)
    {
        BlitSpriteAt(v, pal, kPaper);
        nearY = kHandRestY;
        farY = kHandUpY;
    }
    DrawHand(v, pal, kNearHandX, nearY);
    DrawHand(v, pal, kFarHandX, farY);
}

static void DrawScene(const StageView& v)
{
    const StageState& s = g_Stage;
    const ScenePalette& pal = s.military ? kCubicle : kApartment;
    Pose pose = ResolvePose();

    if (true == s.military)
    {
        DrawCubicleRoom(v);
    }
    else
    {
        DrawApartmentRoom(v);
    }

    BlitSpriteAt(v, pal, kChair);
    if (true == pose.bothArmsUp)
    {
        BlitSprite(v, pal, BlitParams{ &kArmRaised, kBackArmOffsetX, 0, false });
        DrawHand(v, pal, kBackHandX, kBackHandY);
    }
    BlitSpriteAt(v, pal, s.military ? kBodyUniform : kBodyHood);
    BlitSprite(v, pal, BlitParams{ s.military ? &kHeadCap : &kHeadHood, pose.headX, pose.headY, s.blinkMs > 0 });

    // Desk and floor
    FillGridRect(v, kDeskX, kDeskY, kGridW - kDeskX, 1, pal.deskEdge);
    FillGridRect(v, kDeskX, kDeskY + 1, kGridW - kDeskX, 1, pal.desk);
    FillGridRect(v, 0, kFloorY, kGridW, kGridH - kFloorY, pal.floor);

    BlitSpriteAt(v, pal, kKeyboard);
    BlitSpriteAt(v, pal, kMonitor);
    BlitSpriteAt(v, pal, kMug);
    if (0 == Step(kSteamMs, kTwoPhase))
    {
        FillGridRect(v, kSteamAX, kSteamAY, 1, 1, pal.steam);
    }
    else
    {
        FillGridRect(v, kSteamBX, kSteamBY, 1, 1, pal.steam);
    }

    DrawArms(v, pal, pose);
    DrawScreenContents(v, pal);

    for (int i = 0; i < kMaxParticles; i++)
    {
        const Particle& p = s.particles[i];
        if (false == p.active)
        {
            continue;
        }
        int gx = static_cast<int>(p.x + 0.5f);
        int gy = static_cast<int>(p.y + 0.5f);
        if (true == p.dollar)
        {
            BlitGlyph(v, kDollarGlyph, static_cast<int>(std::size(kDollarGlyph)), POINT{ gx, gy }, p.color);
        }
        else
        {
            FillGridRect(v, gx, gy, 1, kSweatDropH, p.color);
        }
    }

    if (s.noticeMs > 0 && 0 == Step(kHandsMs, kTwoPhase))
    {
        BlitGlyph(v, kBangGlyph, static_cast<int>(std::size(kBangGlyph)), POINT{ kBangX, kBangY }, pal.accent);
    }
}

// ============================================================================
// Strip Backdrop and Readout
// ============================================================================

static void DrawBackdrop(HDC hdc, RECT rc, int scale)
{
    const StageState& s = g_Stage;
    const ScenePalette& pal = s.military ? kCubicle : kApartment;
    bool sirenFull = (StageMood::Busted == s.mood);
    int w = rc.right - rc.left;
    int h = rc.bottom - rc.top;

    if (true == sirenFull)
    {
        // Alternating pink / blue wash, left and right halves
        bool phase = (0 == Step(kSirenMs, kTwoPhase));
        COLORREF leftTint = phase ? kNeonPink : kNeonBlue;
        COLORREF rightTint = phase ? kNeonBlue : kNeonPink;
        RECT left = { rc.left, rc.top, rc.left + w / 2, rc.bottom };
        RECT right = { rc.left + w / 2, rc.top, rc.right, rc.bottom };
        FillVerticalGradient(hdc, left, LerpColor(pal.bgTop, leftTint, kSirenTint), LerpColor(pal.bgBottom, leftTint, kSirenTint), kGradientBands);
        FillVerticalGradient(hdc, right, LerpColor(pal.bgTop, rightTint, kSirenTint), LerpColor(pal.bgBottom, rightTint, kSirenTint), kGradientBands);
    }
    else
    {
        FillVerticalGradient(hdc, rc, pal.bgTop, pal.bgBottom, kGradientBands);
    }

    if (false == s.military)
    {
        int cols = w / scale;
        for (int i = 0; i < kRainColumns; i++)
        {
            const RainDrop& d = s.rain[i];
            int gx = static_cast<int>(d.xFrac * static_cast<float>(cols));
            int headY = static_cast<int>(d.y);
            for (int k = 0; k < d.len; k++)
            {
                int gy = headY - k;
                if (gy < 0 || gy * scale >= h)
                {
                    continue;
                }
                COLORREF c = (0 == k) ? (d.pink ? kNeonPinkDim : kNeonBlueDim) : kNeonBlueDark;
                FillPixelRect(hdc, RECT{ rc.left + gx * scale, rc.top + gy * scale, rc.left + (gx + 1) * scale, rc.top + (gy + 1) * scale }, c);
            }
        }
    }
    else
    {
        // Drop ceiling grid
        for (int x = 0; x < w; x += kCeilingTileW * scale)
        {
            FillPixelRect(hdc, RECT{ rc.left + x, rc.top, rc.left + x + 1, rc.bottom }, kCeilingLine);
        }
        for (int y = 0; y < h; y += kCeilingTileH * scale)
        {
            FillPixelRect(hdc, RECT{ rc.left, rc.top + y, rc.right, rc.top + y + 1 }, kCeilingLine);
        }
    }

    // Top edge line, pulsing like a siren when the heat is high
    COLORREF edge = pal.edge;
    if (s.heat >= kSirenHeat || true == sirenFull)
    {
        edge = (0 == Step(kSirenMs, kTwoPhase)) ? kNeonPink : kNeonBlue;
    }
    FillPixelRect(hdc, RECT{ rc.left, rc.top, rc.right, rc.top + kEdgeLineHeight }, edge);
}

static void BuildCaption(const GameState* gs, wchar_t* buf, size_t bufSize, bool withPrompt)
{
    const StageState& s = g_Stage;
    const wchar_t* prompt = withPrompt ? L"> " : L"";
    bool cursor = (0 == Step(kCursorMs, kTwoPhase));
    const wchar_t* tail = cursor ? L"_" : L" ";

    if (true == s.military)
    {
        switch (s.mood)
        {
            case StageMood::Typing:
            {
                int day = gs->militaryDaysCompleted + 1;
                if (day > kMilitaryTotalDays)
                {
                    day = kMilitaryTotalDays;
                }
                StringCchPrintfW(buf, bufSize, L"%son duty. day %d of %d%s", prompt, day, kMilitaryTotalDays, tail);
                return;
            }
            case StageMood::Reading:   StringCchPrintfW(buf, bufSize, L"%smandatory training%s", prompt, tail); return;
            case StageMood::Celebrate: StringCchPrintfW(buf, bufSize, L"%sPAYCHECK", prompt); return;
            case StageMood::Alarm:     StringCchPrintfW(buf, bufSize, L"%s!! INSPECTION !!", prompt); return;
            case StageMood::Busted:    StringCchPrintfW(buf, bufSize, L"%sCONTRACTOR BURN", prompt); return;
            default:                   StringCchPrintfW(buf, bufSize, L"%son duty. yes sir%s", prompt, tail); return;
        }
    }

    switch (s.mood)
    {
        case StageMood::Typing:
            StringCchPrintfW(buf, bufSize, L"%sworking %d %s%s", prompt, s.activeJobs, (1 == s.activeJobs) ? L"job" : L"jobs", tail);
            return;
        case StageMood::Reading:   StringCchPrintfW(buf, bufSize, L"%sstudying the docs%s", prompt, tail); return;
        case StageMood::Celebrate: StringCchPrintfW(buf, bufSize, L"%sPAYDAY", prompt); return;
        case StageMood::Alarm:     StringCchPrintfW(buf, bufSize, L"%s!! HEAT !!", prompt); return;
        case StageMood::Busted:    StringCchPrintfW(buf, bufSize, L"%sSEIZED BY THE FEDS", prompt); return;
        default:                   StringCchPrintfW(buf, bufSize, L"%sidle. waiting for work%s", prompt, tail); return;
    }
}

static COLORREF CaptionColor(const ScenePalette& pal)
{
    const StageState& s = g_Stage;
    if (StageMood::Alarm == s.mood || StageMood::Busted == s.mood)
    {
        return kAlarmRed;
    }
    if (StageMood::Celebrate == s.mood)
    {
        return pal.payday;
    }
    return pal.caption;
}

static const wchar_t* CurrentMutter()
{
    const StageState& s = g_Stage;
    const MutterPool& pool = (s.military ? kMilitaryMutters : kMutters)[static_cast<int>(s.mood)];
    return pool.lines[s.mutterIndex % pool.count];
}

static void DrawReadout(const DrawCtx& ctx, const GameState* gs, RECT rc)
{
    const StageState& s = g_Stage;
    const ScenePalette& pal = s.military ? kCubicle : kApartment;

    FillPixelRect(ctx.hdc, rc, kReadoutFill);
    HPEN pen = CreatePen(PS_SOLID, 1, pal.edge);
    HPEN oldPen = static_cast<HPEN>(SelectObject(ctx.hdc, pen));
    HBRUSH oldBrush = static_cast<HBRUSH>(SelectObject(ctx.hdc, GetStockObject(NULL_BRUSH)));
    Rectangle(ctx.hdc, rc.left, rc.top, rc.right, rc.bottom);
    SelectObject(ctx.hdc, oldBrush);
    SelectObject(ctx.hdc, oldPen);
    DeleteObject(pen);

    // Clip text to the box so long event lines never spill out
    HRGN clip = CreateRectRgn(rc.left + 1, rc.top + 1, rc.right - 1, rc.bottom - 1);
    SelectClipRgn(ctx.hdc, clip);

    auto caption = std::make_unique<wchar_t[]>(BUF_MEDIUM);
    BuildCaption(gs, caption.get(), BUF_MEDIUM, true);
    DrawTextLine(ctx.hdc, TextParams{ ctx.fonts->titleFont, caption.get(), CaptionColor(pal) }, POINT{ rc.left + kReadoutTextX, rc.top + kReadoutCaptionY });

    auto mutter = std::make_unique<wchar_t[]>(BUF_LARGE);
    StringCchPrintfW(mutter.get(), BUF_LARGE, L"\"%s\"", CurrentMutter());
    DrawTextLine(ctx.hdc, TextParams{ ctx.fonts->bodyFont, mutter.get(), pal.mutter }, POINT{ rc.left + kReadoutTextX, rc.top + kReadoutMutterY });

    auto last = std::make_unique<wchar_t[]>(BUF_XLARGE + BUF_MEDIUM);
    const wchar_t* lastText = L"--";
    if (gs->eventLogCount > 0)
    {
        int idx = (gs->eventLogHead - 1 + MAX_EVENT_LOG) % MAX_EVENT_LOG;
        lastText = gs->eventLog[idx].text;
    }
    StringCchPrintfW(last.get(), BUF_XLARGE + BUF_MEDIUM, L"last: %s   |   heat %d%%", lastText, gs->heat);
    DrawTextLine(ctx.hdc, TextParams{ ctx.fonts->smallFont, last.get(), pal.dim }, POINT{ rc.left + kReadoutTextX, rc.top + kReadoutLastY });

    SelectClipRgn(ctx.hdc, NULL);
    DeleteObject(clip);
}

// ============================================================================
// Public Drawing Entry Points
// ============================================================================

void DrawHackerStage(const DrawCtx& ctx, const GameState* gs, PanelRect rc)
{
    const StageState& s = g_Stage;
    if (false == s.initialized)
    {
        return;
    }
    const ScenePalette& pal = s.military ? kCubicle : kApartment;
    RECT area = { rc.x, rc.y, rc.x + rc.w, rc.y + rc.h };

    DrawBackdrop(ctx.hdc, area, kStripScale);

    int sceneX = rc.x + kStripSceneX;
    int sceneY = rc.y + kStripSceneY;
    if (false == s.military)
    {
        // Neon horizon at desk height across the whole strip
        int horizonY = sceneY + kDeskY * kStripScale;
        FillPixelRect(ctx.hdc, RECT{ area.left, horizonY, area.right, horizonY + 1 }, kNeonPinkDim);
        FillPixelRect(ctx.hdc, RECT{ area.left, horizonY + 1, area.right, horizonY + 1 + kHorizonGlowH }, LerpColor(pal.bgBottom, kNeonPink, kHorizonTint));
    }

    DrawScene(StageView{ ctx.hdc, sceneX, sceneY, kStripScale });

    int readoutX = sceneX + kGridW * kStripScale + kReadoutGap;
    RECT readout = { readoutX, rc.y + kReadoutMargin, area.right - kReadoutRight, area.bottom - kReadoutMargin };
    if (readout.right - readout.left > kReadoutTextX * 2)
    {
        DrawReadout(ctx, gs, readout);
    }

    // Bottom edge separating the stage from the panel
    FillPixelRect(ctx.hdc, RECT{ area.left, area.bottom - 1, area.right, area.bottom }, pal.edge);
}

void DrawHackerStageCompact(const DrawCtx& ctx, const GameState* gs, PanelRect rc)
{
    const StageState& s = g_Stage;
    if (false == s.initialized)
    {
        return;
    }
    const ScenePalette& pal = s.military ? kCubicle : kApartment;
    RECT area = { rc.x, rc.y, rc.x + rc.w, rc.y + rc.h };

    DrawBackdrop(ctx.hdc, area, kCompactScale);
    DrawScene(StageView{ ctx.hdc, rc.x + kCompactSceneX, rc.y + kCompactSceneY, kCompactScale });

    HRGN clip = CreateRectRgn(rc.x + kCompactTextX, area.top, area.right - kCompactTextRight, area.bottom);
    SelectClipRgn(ctx.hdc, clip);

    auto caption = std::make_unique<wchar_t[]>(BUF_MEDIUM);
    BuildCaption(gs, caption.get(), BUF_MEDIUM, false);
    DrawTextLine(ctx.hdc, TextParams{ ctx.fonts->smallFont, caption.get(), CaptionColor(pal) }, POINT{ rc.x + kCompactTextX, rc.y + kCompactCaptionY });
    DrawTextLine(ctx.hdc, TextParams{ ctx.fonts->smallFont, CurrentMutter(), pal.mutter }, POINT{ rc.x + kCompactTextX, rc.y + kCompactMutterY });

    SelectClipRgn(ctx.hdc, NULL);
    DeleteObject(clip);

    FillPixelRect(ctx.hdc, RECT{ area.left, area.bottom - 1, area.right, area.bottom }, pal.edge);
}
