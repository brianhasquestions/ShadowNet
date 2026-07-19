#include "events.h"
#include "../ui/colors.h"
#include <strsafe.h>
#include <memory>
#include <ctime>

// ============================================================================
// Random Event Constants
// ============================================================================

constexpr int kEventCheckInterval = 10;        // ticks between event checks
constexpr int kEventRollMax = 1000;

// Forum tip
constexpr int kForumTipRollMax = 30;
constexpr int kForumTipGoodChance = 40;        // percent
constexpr int kForumTipGoodMinPayout = 200;
constexpr int kForumTipGoodMaxPayout = 500;
constexpr int kForumTipBadMinPayout = 25;
constexpr int kForumTipBadMaxPayout = 100;

// Law enforcement
constexpr int kLawEnforcementRollMax = 50;
constexpr int kLawEnforcementDelay = 120;

// Rival hacker
constexpr int kRivalRollMax = 60;
constexpr int kRivalMinRep = 200;
constexpr int kRivalMinOPSEC = 20;
constexpr int kRivalMinLossPercent = 5;
constexpr int kRivalMaxLossPercent = 15;

// FBI raid
constexpr int kFBIRollMax = 65;
constexpr int kFBICashLossDivisor = 10;
constexpr int kFBIHeatReduction = 20;
constexpr int kFBIMinArrestTime = 300;
constexpr int kFBIMaxArrestTime = 600;

// Interpol
constexpr int kInterpolRollMax = 67;
constexpr int kInterpolArrChance = 25;         // percent
constexpr int kInterpolCashLossPercent = 30;
constexpr int kInterpolArrestDuration = 1800;

// Data breach news
constexpr int kBreachNewsRollMax = 75;
constexpr int kBreachNewsRollMin = 67;
constexpr float kBreachNewsPriceBoost = 1.5f;
constexpr int kBreachNewsDuration = 600;

// Patch Tuesday
constexpr int kPatchTuesdayRollMax = 80;
constexpr int kPatchTuesdayRollMin = 75;
constexpr int kPatchTuesdayTickInterval = 1800;
constexpr int kPatchTuesdayTickWindow = 10;
constexpr float kPatchTuesdayExploitMult = 0.7f;
constexpr int kPatchTuesdayDuration = 600;

// Crypto crash
constexpr int kCryptoCrashRollMin = 980;
constexpr int kCryptoCrashRollMax = 990;
constexpr float kCryptoCrashMult = 0.5f;
constexpr int kCryptoCrashDuration = 600;

// Crypto boom
constexpr int kCryptoBoomRollMin = 990;
constexpr int kCryptoBoomRollMax = 997;
constexpr float kCryptoBoomMult = 2.0f;
constexpr int kCryptoBoomDuration = 600;

// Whistleblower
constexpr int kWhistleblowerRollMin = 900;
constexpr int kWhistleblowerRollMax = 920;
constexpr int kWhistleblowerMinLoyalty = 40;
constexpr int kWhistleblowerHeatGain = 30;

// Underground alliance
constexpr int kAllianceRollMin = 910;
constexpr int kAllianceRollMax = 920;
constexpr int kAllianceMinRep = 1000;
constexpr float kAlliancePayoutMult = 1.25f;
constexpr int kAllianceDuration = 300;

// Zero-day
constexpr int kZeroDayRoll = 999;
constexpr int kZeroDayMinValue = 100000;
constexpr int kZeroDayMaxValue = 500000;

// Arrest of competitor
constexpr int kCompetitorRollMin = 920;
constexpr int kCompetitorRollMax = 930;
constexpr float kCompetitorPriceBoost = 1.4f;
constexpr int kCompetitorDuration = 600;

// Journalist investigation
constexpr int kJournalistRollMin = 930;
constexpr int kJournalistRollMax = 940;
constexpr int kJournalistMinRep = 500;
constexpr int kJournalistMinHeat = 40;
constexpr int kJournalistHeatGain = 15;
constexpr int kJournalistRepGain = 20;

// Federal Indictment (new - triggers jail)
constexpr int kIndictmentRollMin = 940;
constexpr int kIndictmentRollMax = 945;
constexpr int kIndictmentMinHeat = 90;

// ============================================================================
// Random Event Processing
// ============================================================================

void ProcessRandomEvents(GameState* pGameState)
{
    if (0 != pGameState->totalTicksPlayed % kEventCheckInterval)
    {
        return;
    }

    int roll = GameRandomRange(pGameState, 0, kEventRollMax);

    // Forum Tip
    if (roll < kForumTipRollMax && pGameState->heat < HEAT_WARNING && false == pGameState->pendingForumTip)
    {
        pGameState->pendingForumTip = true;
        bool goodLead = GameRandomRange(pGameState, 0, 100) < kForumTipGoodChance;
        if (true == goodLead)
        {
            pGameState->forumTipPayout = GameRandomRange(pGameState, kForumTipGoodMinPayout, kForumTipGoodMaxPayout);
            AddEventLog(pGameState, L"Forum tip: a trusted contact shared a solid lead. Check Jobs panel.", Colors::Cyan);
        }
        else
        {
            pGameState->forumTipPayout = GameRandomRange(pGameState, kForumTipBadMinPayout, kForumTipBadMaxPayout);
            AddEventLog(pGameState, L"Forum tip: someone shared a lead. Might be worth checking out.", Colors::Comment);
        }
        AddToast(pGameState, L"New forum tip lead available!", Colors::Cyan);
        return;
    }

    // Law Enforcement Investigation
    if (roll < kLawEnforcementRollMax && pGameState->heat > HEAT_DANGER)
    {
        for (int i = 0; i < MAX_ACTIVE_JOBS; i++)
        {
            if (true == pGameState->activeJobs[i].active)
            {
                pGameState->activeJobs[i].remainingSeconds += kLawEnforcementDelay;
                pGameState->activeJobs[i].totalSeconds += kLawEnforcementDelay;
                AddEventLog(pGameState, L"WARNING: Law enforcement investigation! Job delayed by 120s.", Colors::Red);
                AddToast(pGameState, L"Law enforcement investigation!", Colors::Red);
                break;
            }
        }
        return;
    }

    // Rival Hacker Attack
    if (roll < kRivalRollMax && pGameState->heat > HEAT_WARNING && pGameState->reputation > kRivalMinRep)
    {
        if (pGameState->skills[SKILL_OPSEC].level < kRivalMinOPSEC)
        {
            int64_t loss = pGameState->cash * GameRandomRange(pGameState, kRivalMinLossPercent, kRivalMaxLossPercent) / 100;
            pGameState->cash -= loss;
            if (pGameState->cash < 0)
            {
                pGameState->cash = 0;
            }
            auto msg = std::make_unique<wchar_t[]>(BUF_XLARGE);
            StringCchPrintfW(msg.get(), BUF_XLARGE, L"Rival hacker attacked! Lost $%lld.", loss);
            AddEventLog(pGameState, msg.get(), Colors::Red);
            AddToast(pGameState, msg.get(), Colors::Red);
        }
        else
        {
            AddEventLog(pGameState, L"Rival hacker attack blocked by OPSEC!", Colors::Green);
        }
        return;
    }

    // FBI Raid
    if (roll < kFBIRollMax && pGameState->heat > HEAT_CRITICAL)
    {
        int64_t loss = pGameState->cash / kFBICashLossDivisor;
        pGameState->cash -= loss;
        if (pGameState->cash < 0)
        {
            pGameState->cash = 0;
        }
        pGameState->heat -= kFBIHeatReduction;
        if (pGameState->heat < 0)
        {
            pGameState->heat = 0;
        }

        for (int i = 0; i < MAX_PERSONNEL; i++)
        {
            if (false == pGameState->personnel[i].active || true == pGameState->personnel[i].arrested)
            {
                continue;
            }
            pGameState->personnel[i].arrested = true;
            pGameState->personnel[i].arrestTimer = GameRandomRange(pGameState, kFBIMinArrestTime, kFBIMaxArrestTime);
            if (pGameState->personnel[i].assignedJobSlot >= 0 && pGameState->personnel[i].assignedJobSlot < MAX_ACTIVE_JOBS)
            {
                pGameState->activeJobs[pGameState->personnel[i].assignedJobSlot].assignedPersonnelId = -1;
                pGameState->personnel[i].assignedJobSlot = -1;
            }
            auto msg = std::make_unique<wchar_t[]>(BUF_XLARGE);
            StringCchPrintfW(msg.get(), BUF_XLARGE, L"FBI RAID! \"%s\" arrested. Lost $%lld.", pGameState->personnel[i].name, loss);
            AddEventLog(pGameState, msg.get(), Colors::Red);
            AddToast(pGameState, msg.get(), Colors::Red);
            break;
        }
        return;
    }

    // Interpol Dragnet
    if (roll < kInterpolRollMax && pGameState->heat > HEAT_EXTREME)
    {
        if (GameRandomRange(pGameState, 0, 100) < kInterpolArrChance)
        {
            int64_t loss = pGameState->cash * kInterpolCashLossPercent / 100;
            pGameState->cash -= loss;
            if (pGameState->cash < 0)
            {
                pGameState->cash = 0;
            }
            for (int i = 0; i < MAX_PERSONNEL; i++)
            {
                if (false == pGameState->personnel[i].active || true == pGameState->personnel[i].arrested)
                {
                    continue;
                }
                pGameState->personnel[i].arrested = true;
                pGameState->personnel[i].arrestTimer = kInterpolArrestDuration;
                if (pGameState->personnel[i].assignedJobSlot >= 0 && pGameState->personnel[i].assignedJobSlot < MAX_ACTIVE_JOBS)
                {
                    pGameState->activeJobs[pGameState->personnel[i].assignedJobSlot].assignedPersonnelId = -1;
                    pGameState->personnel[i].assignedJobSlot = -1;
                }
            }
            AddEventLog(pGameState, L"INTERPOL DRAGNET! All personnel arrested! Massive losses!", Colors::Red);
            AddToast(pGameState, L"INTERPOL DRAGNET!", Colors::Red);

            // Interpol dragnet also triggers federal seizure
            if (false == pGameState->jailed)
            {
                pGameState->jailed = true;
                pGameState->jailEndTimestamp = static_cast<int64_t>(time(NULL)) + JAIL_DURATION_SECONDS;
                pGameState->triggerJail = true;
                AddEventLog(pGameState, L"Your operation has been seized. All activities suspended.", Colors::Red);
            }
        }
        return;
    }

    // Data Breach in News
    if (roll < kBreachNewsRollMax && roll >= kBreachNewsRollMin)
    {
        pGameState->marketPriceBoost = kBreachNewsPriceBoost;
        pGameState->marketPriceBoostTimer = kBreachNewsDuration;
        AddEventLog(pGameState, L"Data breach in the news! Data dump prices spiked 50%.", Colors::Yellow);
        AddToast(pGameState, L"Data breach news: prices spiked!", Colors::Yellow);
        return;
    }

    // Exploit Patch Tuesday
    if (roll < kPatchTuesdayRollMax && roll >= kPatchTuesdayRollMin &&
        pGameState->totalTicksPlayed % kPatchTuesdayTickInterval < kPatchTuesdayTickWindow)
    {
        pGameState->exploitPriceMultiplier = kPatchTuesdayExploitMult;
        pGameState->exploitPriceTimer = kPatchTuesdayDuration;
        AddEventLog(pGameState, L"Patch Tuesday: exploit prices dropped 30%, jobs take 20% longer.", Colors::Orange);
        return;
    }

    // Crypto Market Crash
    if (roll >= kCryptoCrashRollMin && roll < kCryptoCrashRollMax)
    {
        pGameState->cryptoPayoutMultiplier = kCryptoCrashMult;
        pGameState->cryptoPayoutTimer = kCryptoCrashDuration;
        AddEventLog(pGameState, L"Crypto market crash! Crypto-related payouts halved for 10 minutes.", Colors::Red);
        AddToast(pGameState, L"Crypto crash!", Colors::Red);
        return;
    }

    // Crypto Market Boom
    if (roll >= kCryptoBoomRollMin && roll < kCryptoBoomRollMax)
    {
        pGameState->cryptoPayoutMultiplier = kCryptoBoomMult;
        pGameState->cryptoPayoutTimer = kCryptoBoomDuration;
        AddEventLog(pGameState, L"Crypto market boom! Crypto-related payouts doubled for 10 minutes!", Colors::Green);
        AddToast(pGameState, L"Crypto boom!", Colors::Green);
        return;
    }

    // Whistleblower / Personnel Betrayal
    if (roll >= kWhistleblowerRollMin && roll < kWhistleblowerRollMax)
    {
        for (int i = 0; i < MAX_PERSONNEL; i++)
        {
            if (true == pGameState->personnel[i].active && pGameState->personnel[i].loyalty < kWhistleblowerMinLoyalty)
            {
                pGameState->heat += kWhistleblowerHeatGain;
                if (pGameState->heat > HEAT_MAX)
                {
                    pGameState->heat = HEAT_MAX;
                }

                pGameState->triggerScreenMelt = true;
                StringCchCopyW(pGameState->meltAttackerName, _countof(pGameState->meltAttackerName), pGameState->personnel[i].name);

                auto msg = std::make_unique<wchar_t[]>(BUF_XLARGE);
                StringCchPrintfW(msg.get(), BUF_XLARGE, L"BETRAYAL! \"%s\" turned on you and hacked your systems! Heat +30!", pGameState->personnel[i].name);
                AddEventLog(pGameState, msg.get(), Colors::Red);
                AddToast(pGameState, msg.get(), Colors::Red);
                pGameState->personnel[i].active = false;
                break;
            }
        }
        return;
    }

    // Underground Alliance
    if (roll >= kAllianceRollMin && roll < kAllianceRollMax && pGameState->reputation > kAllianceMinRep)
    {
        pGameState->payoutMultiplier = kAlliancePayoutMult;
        pGameState->payoutMultiplierTimer = kAllianceDuration;
        AddEventLog(pGameState, L"Underground Alliance! 25% payout boost for 5 minutes.", Colors::Pink);
        AddToast(pGameState, L"Underground Alliance active!", Colors::Pink);
        return;
    }

    // Zero-Day Dropped
    if (kZeroDayRoll == roll)
    {
        pGameState->inventory.exploits++;
        int64_t value = GameRandomRange(pGameState, kZeroDayMinValue, kZeroDayMaxValue);
        pGameState->cash += value;
        auto msg = std::make_unique<wchar_t[]>(BUF_XLARGE);
        StringCchPrintfW(msg.get(), BUF_XLARGE, L"RARE: Zero-day dropped! Free exploit worth $%lld!", value);
        AddEventLog(pGameState, msg.get(), Colors::Yellow);
        AddToast(pGameState, msg.get(), Colors::Yellow);
        return;
    }

    // Arrest of Competitor
    if (roll >= kCompetitorRollMin && roll < kCompetitorRollMax)
    {
        pGameState->marketPriceBoost = kCompetitorPriceBoost;
        pGameState->marketPriceBoostTimer = kCompetitorDuration;
        AddEventLog(pGameState, L"Competitor arrested! Market monopoly: prices +40% for 10 minutes.", Colors::Green);
        return;
    }

    // Journalist Investigation
    if (roll >= kJournalistRollMin && roll < kJournalistRollMax &&
        pGameState->reputation > kJournalistMinRep && pGameState->heat > kJournalistMinHeat)
    {
        pGameState->heat += kJournalistHeatGain;
        pGameState->reputation += kJournalistRepGain;
        if (pGameState->heat > HEAT_MAX)
        {
            pGameState->heat = HEAT_MAX;
        }
        AddEventLog(pGameState, L"Journalist investigation! Heat +15, Rep +20. Any publicity is good publicity.", Colors::Orange);
        return;
    }

    // Federal Indictment - triggers jail at extreme heat
    if (roll >= kIndictmentRollMin && roll < kIndictmentRollMax &&
        pGameState->heat > kIndictmentMinHeat && false == pGameState->jailed)
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

        AddEventLog(pGameState, L"FEDERAL INDICTMENT! A grand jury has issued a warrant for your arrest.", Colors::Red);
        AddEventLog(pGameState, L"Your operation has been seized. All activities suspended for 24 hours.", Colors::Red);
        AddToast(pGameState, L"FEDERAL INDICTMENT!", Colors::Red);
        return;
    }
}
