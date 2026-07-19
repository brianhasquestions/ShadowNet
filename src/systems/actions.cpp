#include "actions.h"
#include "../ui/colors.h"
#include "../ui/ui.h"
#include "../data/strings.h"
#include <strsafe.h>
#include <memory>

// ============================================================================
// Personnel Hiring Constants
// ============================================================================

constexpr int kBaseLoyalty = 70;
constexpr int kLoyaltyRange = 30;
constexpr int kScamHireThreshold = 3;
constexpr int kScamHeatGainBase = 5;
constexpr int kScamHeatGainPerThousand = 1000;
constexpr int kScamRepLossDivisor = 100;
constexpr int kScamMeltExtraHeat = 20;
constexpr int kScamMeltRepLoss = 10;

// ============================================================================
// Personnel Hiring Logic
// ============================================================================

void HirePersonnel(GameState* pGameState, int typeIndex, SpawnIRCWindowFunc spawnIRC, HWND hMainWnd,
                   SpawnScreenMeltFunc spawnMelt)
{
    PersonnelType type = static_cast<PersonnelType>(typeIndex);
    const PersonnelTypeDef& def = g_PersonnelTypes[typeIndex];

    int personnelCount = 0;
    for (int i = 0; i < MAX_PERSONNEL; i++)
    {
        if (true == pGameState->personnel[i].active)
        {
            personnelCount++;
        }
    }
    if (personnelCount >= pGameState->maxPersonnelSlots)
    {
        return;
    }
    if (pGameState->reputation < def.requiredRep)
    {
        return;
    }

    bool canAfford = (pGameState->cash >= def.cost);

    if (true == canAfford)
    {
        pGameState->cash -= def.cost;

        int slot = -1;
        for (int i = 0; i < MAX_PERSONNEL; i++)
        {
            if (false == pGameState->personnel[i].active)
            {
                slot = i;
                break;
            }
        }
        if (slot < 0)
        {
            return;
        }

        PersonnelMember& pm = pGameState->personnel[slot];
        pm.active = true;
        pm.id = slot;
        pm.type = type;
        pm.level = 1;
        pm.loyalty = kBaseLoyalty + GameRandomRange(pGameState, 0, kLoyaltyRange);
        pm.assignedJobSlot = -1;
        pm.arrested = false;
        pm.arrestTimer = 0;

        int pIdx = GameRandomRange(pGameState, 0, NUM_PREFIXES - 1);
        int sIdx = GameRandomRange(pGameState, 0, NUM_SUFFIXES - 1);
        StringCchPrintfW(pm.name, _countof(pm.name), L"%s%s", g_HandlePrefixes[pIdx], g_HandleSuffixes[sIdx]);

        auto msg = std::make_unique<wchar_t[]>(BUF_LARGE);
        StringCchPrintfW(msg.get(), BUF_LARGE, L"Hired %s \"%s\".", def.name, pm.name);
        AddEventLog(pGameState, msg.get(), Colors::Cyan);
        AddToast(pGameState, msg.get(), Colors::Cyan);
        spawnIRC(pGameState, hMainWnd, typeIndex, true, pm.name);
    }
    else
    {
        pGameState->scamHireCount++;

        int64_t stolenCash = pGameState->cash;
        pGameState->cash = 0;
        int repLoss = static_cast<int>(def.cost / kScamRepLossDivisor);
        if (repLoss > pGameState->reputation)
        {
            repLoss = pGameState->reputation;
        }
        pGameState->reputation -= repLoss;
        int heatGain = kScamHeatGainBase + static_cast<int>(def.cost / kScamHeatGainPerThousand);
        pGameState->heat += heatGain;
        if (pGameState->heat > HEAT_MAX)
        {
            pGameState->heat = HEAT_MAX;
        }

        int nameIdx = GameRandomRange(pGameState, 0, NUM_HACKER_USERNAMES - 1);
        const wchar_t* scammerHandle = g_HackerUsernames[nameIdx];

        if (pGameState->scamHireCount >= kScamHireThreshold)
        {
            auto msg = std::make_unique<wchar_t[]>(BUF_XLARGE);
            StringCchPrintfW(msg.get(), BUF_XLARGE, L"You wasted \"%s\"'s time one too many times. They're coming for you.", scammerHandle);
            AddEventLog(pGameState, msg.get(), Colors::Red);
            AddToast(pGameState, L"You just pissed off the wrong person.", Colors::Red);

            pGameState->triggerScreenMelt = true;
            StringCchCopyW(pGameState->meltAttackerName, _countof(pGameState->meltAttackerName), scammerHandle);

            pGameState->heat += kScamMeltExtraHeat;
            if (pGameState->heat > HEAT_MAX)
            {
                pGameState->heat = HEAT_MAX;
            }
            pGameState->reputation -= kScamMeltRepLoss;
            if (pGameState->reputation < 0)
            {
                pGameState->reputation = 0;
            }

            pGameState->scamHireCount = 0;
            spawnIRC(pGameState, hMainWnd, typeIndex, false, scammerHandle);
        }
        else
        {
            auto msg = std::make_unique<wchar_t[]>(BUF_XLARGE);
            if (1 == pGameState->scamHireCount)
            {
                StringCchPrintfW(msg.get(), BUF_XLARGE, L"SCAM! \"%s\" took your money. -%d rep, +%d heat.", scammerHandle, repLoss, heatGain);
            }
            else
            {
                StringCchPrintfW(msg.get(), BUF_XLARGE, L"SCAM AGAIN! \"%s\" is getting annoyed. Keep this up and someone's going to make you pay.", scammerHandle);
            }
            AddEventLog(pGameState, msg.get(), Colors::Red);
            AddToast(pGameState, msg.get(), Colors::Red);
            spawnIRC(pGameState, hMainWnd, typeIndex, false, scammerHandle);
        }
    }
}

// ============================================================================
// Market Buy Logic
// ============================================================================

void BuyMarketItem(GameState* pGameState, int itemIndex)
{
    if (itemIndex < 0 || itemIndex >= g_NumMarketItems)
    {
        return;
    }

    const MarketItem& item = g_MarketItems[itemIndex];
    int64_t price = static_cast<int64_t>(item.basePrice * pGameState->market.priceMultipliers[static_cast<int>(item.type)]);

    if (pGameState->cash < price)
    {
        return;
    }
    pGameState->cash -= price;

    switch (item.type)
    {
        case MarketItemType::VPNService:
            pGameState->inventory.hasVPN = true;
            break;
        case MarketItemType::ProxyChain:
            pGameState->inventory.hasProxyChain = true;
            break;
        case MarketItemType::BurnerPhone:
            pGameState->inventory.burnerPhones++;
            break;
        case MarketItemType::HardwareDevKit:
            pGameState->inventory.hasHardwareDevKit = true;
            break;
        case MarketItemType::ZeroDayKit:
            pGameState->inventory.hasZeroDayKit = true;
            break;
        case MarketItemType::CustomC2:
            pGameState->inventory.hasCustomC2 = true;
            break;
        case MarketItemType::DedicatedServer:
            pGameState->inventory.dedicatedServers++;
            break;
        case MarketItemType::BulletproofVPS:
            pGameState->inventory.bulletproofVPS++;
            break;
        case MarketItemType::TorHiddenService:
            pGameState->inventory.hasTorHiddenService = true;
            break;
        case MarketItemType::SatelliteUplink:
            pGameState->inventory.hasSatelliteUplink = true;
            break;
        default:
            break;
    }

    auto msg = std::make_unique<wchar_t[]>(BUF_LARGE);
    auto priceBuf = std::make_unique<wchar_t[]>(BUF_MEDIUM);
    FormatCurrency(price, priceBuf.get(), BUF_MEDIUM);
    StringCchPrintfW(msg.get(), BUF_LARGE, L"Purchased %s for %s.", item.name, priceBuf.get());
    AddEventLog(pGameState, msg.get(), Colors::Green);
    AddToast(pGameState, msg.get(), Colors::Green);
}

// ============================================================================
// Market Sell Logic
// ============================================================================

void SellMarketItem(GameState* pGameState, int itemIndex)
{
    if (itemIndex < 0 || itemIndex >= g_NumMarketItems)
    {
        return;
    }

    const MarketItem& item = g_MarketItems[itemIndex];
    if (true == item.isBuyable)
    {
        return;
    }

    // Resolve the backing inventory counter for this sellable good.
    int* pCount = nullptr;
    switch (item.type)
    {
        case MarketItemType::DataDump: pCount = &pGameState->inventory.dataDumps; break;
        case MarketItemType::Exploit:  pCount = &pGameState->inventory.exploits;  break;
        default: return;
    }
    if (nullptr == pCount || *pCount <= 0)
    {
        return;
    }

    int64_t price = static_cast<int64_t>(item.basePrice * pGameState->market.priceMultipliers[static_cast<int>(item.type)]);
    (*pCount)--;
    pGameState->cash += price;
    pGameState->totalEarned += price;

    auto msg = std::make_unique<wchar_t[]>(BUF_LARGE);
    auto priceBuf = std::make_unique<wchar_t[]>(BUF_MEDIUM);
    FormatCurrency(price, priceBuf.get(), BUF_MEDIUM);
    StringCchPrintfW(msg.get(), BUF_LARGE, L"Sold %s for %s.", item.name, priceBuf.get());
    AddEventLog(pGameState, msg.get(), Colors::Green);
    AddToast(pGameState, msg.get(), Colors::Green);
}

// ============================================================================
// Prestige Logic
// ============================================================================

void ActivatePrestige(GameState* pGameState)
{
    if (false == ArePrestigeRequirementsMet(pGameState))
    {
        return;
    }

    pGameState->prestigeLevel++;

    pGameState->cash = pGameState->cash / PRESTIGE_CASH_RATIO;

    pGameState->reputation = 0;
    pGameState->heat = 0;
    pGameState->inPrestigeMode = true;

    for (int i = 0; i < NUM_SKILLS; i++)
    {
        pGameState->skills[i].level = 0;
        pGameState->skills[i].xp = 0;
    }

    for (int i = 0; i < MAX_ACTIVE_JOBS; i++)
    {
        pGameState->activeJobs[i].active = false;
    }

    int jobCount = GetJobCount();
    ZeroMemory(pGameState->jobCompletionCounts, sizeof(pGameState->jobCompletionCounts));

    for (int i = 0; i < jobCount; i++)
    {
        const JobDef& job = GetJobDef(i);
        pGameState->jobUnlocked[i] = (true == job.isLegitimate && Tier::Four == job.tier);
    }

    for (int i = 0; i < MAX_PERSONNEL; i++)
    {
        pGameState->personnel[i].assignedJobSlot = -1;
    }

    AddEventLog(pGameState, L"PRESTIGE! You've gone legitimate.", Colors::Pink);
    AddEventLog(pGameState, L"Welcome to the other side of the firewall.", Colors::Pink);
    AddToast(pGameState, L"PRESTIGE ACTIVATED!", Colors::Pink);
}
