#include "intel.h"

// ============================================================================
// Intel Reduction Constants
// ============================================================================

constexpr float kLinkedInReduction = 0.10f;
constexpr float kParkMobileReduction = 0.08f;
constexpr float kNPDReduction = 0.15f;
constexpr float kAutoZoneReduction = 0.12f;
constexpr float kMGMReduction = 0.15f;
constexpr float kTMobileReduction = 0.12f;
constexpr float kLastPassReduction = 0.20f;
constexpr float kMOVEitReduction = 0.15f;
constexpr float kEquifaxReduction = 0.15f;
constexpr float kAnthemReduction = 0.12f;
constexpr float kFacebookReduction = 0.10f;
constexpr float kTwitterReduction = 0.08f;
constexpr float kCorporateVPNReduction = 0.20f;
constexpr float kRDPReduction = 0.15f;
constexpr float kStolenIdentityReduction = 0.10f;
constexpr float kEvilNginxReduction = 0.15f;
constexpr float kPhishingKitReduction = 0.10f;
constexpr float kExploitPackReduction = 0.10f;
constexpr float kMaxIntelReduction = 0.40f;

// ============================================================================
// Intel Market Constants
// ============================================================================

constexpr int kMinIntelStock = 1;
constexpr int kMaxIntelStock = 5;
constexpr int kMinPriceFluctuation = -20;
constexpr int kMaxPriceFluctuation = 20;
constexpr float kFluctuationDivisor = 100.0f;
constexpr float kMinPriceMultiplier = 0.5f;
constexpr float kMaxPriceMultiplier = 2.0f;

// ============================================================================
// Intel Time Reduction
// ============================================================================

float GetIntelTimeReduction(const GameState* pGameState, int jobDefId)
{
    const JobDef& job = GetJobDef(jobDefId);
    float reduction = 0.0f;

    auto trainsSkill = [&](SkillId target) -> bool
    {
        for (int i = 0; i < job.numSkillsTrained; i++)
        {
            if (job.skillsTrained[i].skill == target)
            {
                return true;
            }
        }
        return false;
    };

    // Data Dumps
    if (pGameState->intelInventory[static_cast<int>(IntelItemType::LinkedInCombo)] > 0 && true == trainsSkill(SKILL_DATABASE_SYSTEMS))
    {
        reduction += kLinkedInReduction;
    }
    if (pGameState->intelInventory[static_cast<int>(IntelItemType::ParkMobileBreach)] > 0 && true == trainsSkill(SKILL_SOCIAL_ENGINEERING))
    {
        reduction += kParkMobileReduction;
    }
    if (pGameState->intelInventory[static_cast<int>(IntelItemType::NationalPublicData)] > 0 &&
        (true == trainsSkill(SKILL_SOCIAL_ENGINEERING) || true == trainsSkill(SKILL_FINANCIAL_SYSTEMS)))
    {
        reduction += kNPDReduction;
    }
    if (pGameState->intelInventory[static_cast<int>(IntelItemType::AutoZoneFullz)] > 0 && true == trainsSkill(SKILL_FINANCIAL_SYSTEMS))
    {
        reduction += kAutoZoneReduction;
    }
    if (pGameState->intelInventory[static_cast<int>(IntelItemType::MGMResortsDump)] > 0 && true == trainsSkill(SKILL_SOCIAL_ENGINEERING))
    {
        reduction += kMGMReduction;
    }
    if (pGameState->intelInventory[static_cast<int>(IntelItemType::TMobileCustomerDB)] > 0 && true == trainsSkill(SKILL_TELECOMMUNICATIONS))
    {
        reduction += kTMobileReduction;
    }
    if (pGameState->intelInventory[static_cast<int>(IntelItemType::LastPassVaultData)] > 0 && true == trainsSkill(SKILL_DATABASE_SYSTEMS))
    {
        reduction += kLastPassReduction;
    }
    if (pGameState->intelInventory[static_cast<int>(IntelItemType::MOVEitTransferDump)] > 0 &&
        (true == trainsSkill(SKILL_OPSEC) || true == trainsSkill(SKILL_NETWORK_ENGINEERING)))
    {
        reduction += kMOVEitReduction;
    }
    if (pGameState->intelInventory[static_cast<int>(IntelItemType::EquifaxCreditData)] > 0 && true == trainsSkill(SKILL_FINANCIAL_SYSTEMS))
    {
        reduction += kEquifaxReduction;
    }
    if (pGameState->intelInventory[static_cast<int>(IntelItemType::AnthemHealthRecords)] > 0 && true == trainsSkill(SKILL_SOCIAL_ENGINEERING))
    {
        reduction += kAnthemReduction;
    }
    if (pGameState->intelInventory[static_cast<int>(IntelItemType::FacebookMetaLeak)] > 0 && true == trainsSkill(SKILL_SOCIAL_ENGINEERING))
    {
        reduction += kFacebookReduction;
    }
    if (pGameState->intelInventory[static_cast<int>(IntelItemType::TwitterXUserData)] > 0 && true == trainsSkill(SKILL_SOCIAL_ENGINEERING))
    {
        reduction += kTwitterReduction;
    }

    // Access items
    if (pGameState->intelInventory[static_cast<int>(IntelItemType::CorporateVPNAccess)] > 0 && true == trainsSkill(SKILL_NETWORK_ENGINEERING))
    {
        reduction += kCorporateVPNReduction;
    }
    if (pGameState->intelInventory[static_cast<int>(IntelItemType::RDPAccess)] > 0 && true == trainsSkill(SKILL_WINDOWS_EXPLOITATION))
    {
        reduction += kRDPReduction;
    }

    // Identity
    if (pGameState->intelInventory[static_cast<int>(IntelItemType::StolenIdentity)] > 0 && true == trainsSkill(SKILL_SOCIAL_ENGINEERING))
    {
        reduction += kStolenIdentityReduction;
    }

    // Tools
    if (pGameState->intelInventory[static_cast<int>(IntelItemType::EvilNginxConfig)] > 0 && true == trainsSkill(SKILL_SOCIAL_ENGINEERING))
    {
        reduction += kEvilNginxReduction;
    }
    if (pGameState->intelInventory[static_cast<int>(IntelItemType::PhishingKit)] > 0 && true == trainsSkill(SKILL_SOCIAL_ENGINEERING))
    {
        reduction += kPhishingKitReduction;
    }
    if (pGameState->intelInventory[static_cast<int>(IntelItemType::ExploitPack)] > 0 && true == trainsSkill(SKILL_REVERSE_ENGINEERING))
    {
        reduction += kExploitPackReduction;
    }

    if (reduction > kMaxIntelReduction)
    {
        reduction = kMaxIntelReduction;
    }
    return reduction;
}

// ============================================================================
// Intel Market Stock Refresh
// ============================================================================

void RefreshIntelMarketStock(GameState* pGameState)
{
    for (int i = 0; i < static_cast<int>(IntelItemType::COUNT); i++)
    {
        pGameState->intelMarketStock[i] = GameRandomRange(pGameState, kMinIntelStock, kMaxIntelStock);
        float change = static_cast<float>(GameRandomRange(pGameState, kMinPriceFluctuation, kMaxPriceFluctuation)) / kFluctuationDivisor;
        pGameState->intelPriceMultipliers[i] *= (1.0f + change);
        if (pGameState->intelPriceMultipliers[i] < kMinPriceMultiplier)
        {
            pGameState->intelPriceMultipliers[i] = kMinPriceMultiplier;
        }
        if (pGameState->intelPriceMultipliers[i] > kMaxPriceMultiplier)
        {
            pGameState->intelPriceMultipliers[i] = kMaxPriceMultiplier;
        }
    }
}
