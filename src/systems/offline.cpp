#include "offline.h"
#include "skills.h"
#include "../ui/colors.h"
#include <strsafe.h>
#include <memory>

// ============================================================================
// Offline Progress Constants
// ============================================================================

constexpr float kOfflinePersonnelPayoutMult = 0.70f;
constexpr int kTorIncomePerMinute = 5;
constexpr int kVPSIncomePerMinute = 3;
constexpr int kServerIncomePerMinute = 2;
constexpr int kSatelliteIncomePerMinute = 10;
constexpr int kSecondsPerMinute = 60;
constexpr int kSecondsPerHour = 3600;

// ============================================================================
// Offline Progress Processing
// ============================================================================

void ProcessOfflineProgress(GameState* pGameState, int64_t elapsed)
{
    if (elapsed <= 0)
    {
        return;
    }

    // If jailed, check if jail time expired during offline period
    if (true == pGameState->jailed)
    {
        int64_t now = static_cast<int64_t>(time(NULL));
        if (now >= pGameState->jailEndTimestamp)
        {
            pGameState->jailed = false;
            pGameState->jailEndTimestamp = 0;
            pGameState->heat = 0;
            AddEventLog(pGameState, L"Released from federal custody while offline.", Colors::Cyan);
        }
        else
        {
            // Still jailed — no offline progress
            pGameState->totalTicksPlayed += elapsed;
            auto msg = std::make_unique<wchar_t[]>(BUF_XLARGE);
            int64_t remaining = pGameState->jailEndTimestamp - now;
            int hours = static_cast<int>(remaining / kSecondsPerHour);
            int minutes = static_cast<int>((remaining % kSecondsPerHour) / kSecondsPerMinute);
            StringCchPrintfW(msg.get(), BUF_XLARGE, L"Still in federal custody. Release in %dh %dm.", hours, minutes);
            AddEventLog(pGameState, msg.get(), Colors::Red);
            return;
        }
    }

    int64_t offlineCash = 0;
    int offlineJobsCompleted = 0;

    // 1. Complete active jobs that were already running
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

        int remaining = pGameState->activeJobs[i].remainingSeconds;
        if (remaining <= static_cast<int>(elapsed))
        {
            const JobDef& job = GetJobDef(pGameState->activeJobs[i].jobDefId);
            float payoutBonus = GetSkillPayoutBonus(pGameState, pGameState->activeJobs[i].jobDefId);
            float personnelMult = 1.0f;
            if (pid >= 0 && pid < MAX_PERSONNEL)
            {
                personnelMult = kOfflinePersonnelPayoutMult;
                SkillId spec = g_PersonnelTypes[static_cast<int>(pGameState->personnel[pid].type)].specialty;
                for (int s = 0; s < job.numSkillsTrained; s++)
                {
                    if (job.skillsTrained[s].skill == spec)
                    {
                        personnelMult = 1.0f;
                        break;
                    }
                }
            }
            int64_t payout = static_cast<int64_t>(job.basePayoutUsd * (1.0f + payoutBonus) * personnelMult);

            pGameState->cash += payout;
            pGameState->totalEarned += payout;
            pGameState->reputation += job.repReward;
            pGameState->totalJobsCompleted++;
            pGameState->jobCompletionCounts[pGameState->activeJobs[i].jobDefId]++;
            offlineCash += payout;
            offlineJobsCompleted++;

            // Award skill XP (simplified offline version - no milestones/toasts)
            for (int s = 0; s < job.numSkillsTrained; s++)
            {
                SkillId sid = job.skillsTrained[s].skill;
                int xpGain = job.skillsTrained[s].xpGain;
                if (xpGain < 1) xpGain = 1;
                pGameState->skills[sid].xp += xpGain;

                constexpr int kMaxSkillLevel = 100;
                while (pGameState->skills[sid].level < kMaxSkillLevel &&
                       pGameState->skills[sid].xp >= XpForNextLevel(pGameState->skills[sid].level))
                {
                    pGameState->skills[sid].xp -= XpForNextLevel(pGameState->skills[sid].level);
                    pGameState->skills[sid].level++;
                }
            }

            // Add heat (reduced offline - no compounding)
            pGameState->heat += job.heatGenerated;
            if (pGameState->heat > HEAT_MAX) pGameState->heat = HEAT_MAX;

            if (pid >= 0 && pid < MAX_PERSONNEL)
            {
                pGameState->personnel[pid].assignedJobSlot = -1;
            }

            pGameState->activeJobs[i].active = false;
            pGameState->activeJobs[i].assignedPersonnelId = -1;
        }
        else
        {
            pGameState->activeJobs[i].remainingSeconds -= static_cast<int>(elapsed);
        }
    }

    // 2. Release arrested personnel
    for (int i = 0; i < MAX_PERSONNEL; i++)
    {
        if (false == pGameState->personnel[i].active)
        {
            continue;
        }
        if (true == pGameState->personnel[i].arrested)
        {
            pGameState->personnel[i].arrestTimer -= static_cast<int>(elapsed);
            if (pGameState->personnel[i].arrestTimer <= 0)
            {
                pGameState->personnel[i].arrested = false;
                pGameState->personnel[i].arrestTimer = 0;
            }
        }
    }

    // 3. Heat decay
    int heatDecays = static_cast<int>(elapsed / HEAT_DECAY_INTERVAL);
    pGameState->heat -= heatDecays;
    if (pGameState->heat < 0)
    {
        pGameState->heat = 0;
    }

    // 4. Passive income from infrastructure
    int64_t passiveIncome = 0;
    if (true == pGameState->inventory.hasTorHiddenService)
    {
        passiveIncome += (elapsed / kSecondsPerMinute) * kTorIncomePerMinute;
    }
    if (pGameState->inventory.bulletproofVPS > 0)
    {
        passiveIncome += (elapsed / kSecondsPerMinute) * pGameState->inventory.bulletproofVPS * kVPSIncomePerMinute;
    }
    if (pGameState->inventory.dedicatedServers > 0)
    {
        passiveIncome += (elapsed / kSecondsPerMinute) * pGameState->inventory.dedicatedServers * kServerIncomePerMinute;
    }
    if (true == pGameState->inventory.hasSatelliteUplink)
    {
        passiveIncome += (elapsed / kSecondsPerMinute) * kSatelliteIncomePerMinute;
    }

    pGameState->cash += passiveIncome;
    pGameState->totalEarned += passiveIncome;
    offlineCash += passiveIncome;

    // 5. Doc study progress
    if (true == pGameState->docStudyActive)
    {
        pGameState->docStudyRemaining -= static_cast<int>(elapsed);
        if (pGameState->docStudyRemaining <= 0)
        {
            pGameState->docStudyActive = false;
            pGameState->docStudyRemaining = 0;
            int docIdx = static_cast<int>(pGameState->currentDocStudy);
            pGameState->docStudyPasses[docIdx]++;
            if (1 == pGameState->docStudyPasses[docIdx])
            {
                pGameState->docsRead[docIdx] = true;
            }
            const DocStudyDef& doc = GetDocDef(docIdx);
            for (int s = 0; s < doc.numSkillBonuses; s++)
            {
                pGameState->skills[doc.skillBonus[s].skill].xp += doc.skillBonus[s].xpGain;
            }
            AddEventLog(pGameState, L"Documentation study completed while offline.", Colors::Purple);
        }
    }

    // 6. Timer decays
    if (pGameState->payoutMultiplierTimer > 0)
    {
        pGameState->payoutMultiplierTimer -= static_cast<int>(elapsed);
        if (pGameState->payoutMultiplierTimer <= 0)
        {
            pGameState->payoutMultiplierTimer = 0;
            pGameState->payoutMultiplier = 1.0f;
        }
    }
    if (pGameState->exploitPriceTimer > 0)
    {
        pGameState->exploitPriceTimer -= static_cast<int>(elapsed);
        if (pGameState->exploitPriceTimer <= 0)
        {
            pGameState->exploitPriceTimer = 0;
            pGameState->exploitPriceMultiplier = 1.0f;
        }
    }
    if (pGameState->skillXPMultiplierTimer > 0)
    {
        pGameState->skillXPMultiplierTimer -= static_cast<int>(elapsed);
        if (pGameState->skillXPMultiplierTimer <= 0)
        {
            pGameState->skillXPMultiplierTimer = 0;
            pGameState->skillXPMultiplier = 1.0f;
        }
    }
    for (int i = 0; i < NUM_BOSS_HACKS; i++)
    {
        if (pGameState->bossAttemptCooldown[i] > 0)
        {
            pGameState->bossAttemptCooldown[i] -= static_cast<int>(elapsed);
            if (pGameState->bossAttemptCooldown[i] < 0)
            {
                pGameState->bossAttemptCooldown[i] = 0;
            }
        }
    }

    // Game hack ban timer
    if (pGameState->gameHackBanTimer > 0)
    {
        pGameState->gameHackBanTimer -= static_cast<int>(elapsed);
        if (pGameState->gameHackBanTimer < 0)
        {
            pGameState->gameHackBanTimer = 0;
        }
    }

    // 8. Military service progress
    if (true == IsMilitaryActive(pGameState))
    {
        int64_t now = static_cast<int64_t>(time(NULL));

        // Process pay periods earned while offline
        int payPeriods = static_cast<int>(elapsed / MILITARY_PAY_INTERVAL);
        if (payPeriods > 0)
        {
            int64_t pay = payPeriods * MILITARY_BASE_PAY;
            pGameState->cash += pay;
            pGameState->totalEarned += pay;
            pGameState->militaryTotalPay += pay;
        }

        // Check how many full days passed
        int64_t totalMilitaryElapsed = now - pGameState->militaryEnlistTimestamp;
        int totalDaysPassed = static_cast<int>(totalMilitaryElapsed / MILITARY_DAY_SECONDS);

        if (MilitaryPhase::Training == pGameState->militaryPhase && totalDaysPassed >= 1)
        {
            // Training day complete - force skills to 65
            for (int i = 0; i < NUM_SKILLS; i++)
            {
                if (pGameState->skills[i].level < MILITARY_SKILL_CAP_TRAINING)
                {
                    pGameState->skills[i].level = MILITARY_SKILL_CAP_TRAINING;
                    pGameState->skills[i].xp = 0;
                }
            }
            pGameState->militaryDaysCompleted = 1;
            pGameState->militaryPhase = MilitaryPhase::SpecialtyPick;
            AddEventLog(pGameState, L"Basic cyber training completed while offline. Choose your specialty.", Colors::Olive);
        }

        if (MilitaryPhase::ActiveDuty == pGameState->militaryPhase && totalDaysPassed >= MILITARY_SERVICE_DAYS)
        {
            CompleteMilitaryService(pGameState);
            AddEventLog(pGameState, L"Military service completed while offline.", Colors::Green);
        }
        else if (MilitaryPhase::ActiveDuty == pGameState->militaryPhase)
        {
            pGameState->militaryDaysCompleted = totalDaysPassed;
            pGameState->militaryDayStartTimestamp = pGameState->militaryEnlistTimestamp + (int64_t)totalDaysPassed * MILITARY_DAY_SECONDS;
        }
    }

    // Update slot counts
    pGameState->maxJobSlots = GetMaxJobSlots(pGameState);
    pGameState->maxPersonnelSlots = GetMaxPersonnelSlots(pGameState);
    pGameState->totalTicksPlayed += elapsed;

    // Welcome back message
    auto msg = std::make_unique<wchar_t[]>(BUF_XLARGE);
    int hours = static_cast<int>(elapsed / kSecondsPerHour);
    int minutes = static_cast<int>((elapsed % kSecondsPerHour) / kSecondsPerMinute);
    if (offlineJobsCompleted > 0 || passiveIncome > 0)
    {
        StringCchPrintfW(msg.get(), BUF_XLARGE, L"Welcome back! %dh %dm offline. %d jobs completed, +$%lld earned.", hours, minutes, offlineJobsCompleted, offlineCash);
    }
    else
    {
        StringCchPrintfW(msg.get(), BUF_XLARGE, L"Welcome back! %dh %dm offline. No active operations completed.", hours, minutes);
    }
    AddEventLog(pGameState, msg.get(), Colors::Cyan);
}
