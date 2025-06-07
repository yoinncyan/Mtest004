// AdvancedSystems.cpp
#include "AdvancedSystems.h"
#include "Engine/World.h" // Required for GetWorld() if not already included via other headers
#include "TimerManager.h" // Required for FTimerManager

// UObliterationSystem Implementation
void UObliterationSystem::CheckOTConditions(AActor* Target)
{
    // TODO: Implement Obliteration Technique condition checking logic
    // Example: UE_LOG(LogTemp, Log, TEXT("Checking OT Conditions for target: %s"), Target ? *Target->GetName() : TEXT("None"));
}

// UUltimateTechSystem Implementation
void UUltimateTechSystem::StartCharging()
{
    if (GetWorld()) // Ensure GetWorld() is valid
    {
        ChargeStartTime = GetWorld()->GetTimeSeconds();
        
        // 蓄力计算公式
        const float BaseChargeTime = 100.0f / 60.0f; // 100帧转换为秒
        const float ActualChargeTime = FMath::Max(
            BaseChargeTime - (EssenceCount * 10.0f / 60.0f),
            50.0f / 60.0f
        );
        
        GetWorld()->GetTimerManager().SetTimer(
            ChargeTimerHandle,
            this, &UUltimateTechSystem::OnChargeComplete,
            ActualChargeTime,
            false // Ensure the timer does not loop by default
        );
        // UE_LOG(LogTemp, Log, TEXT("Ultimate Tech charging started. Duration: %f seconds"), ActualChargeTime);
    }
}

void UUltimateTechSystem::OnChargeComplete()
{
    // TODO: Implement logic for when UT charging is complete
    // Example: UE_LOG(LogTemp, Log, TEXT("Ultimate Tech charge complete!"));
    // bIsCharged = true; // Example flag
}

void UUltimateTechSystem::ReleaseUT()
{
    // TODO: Implement logic for releasing the Ultimate Technique
    // This would typically check if the UT is charged, then execute it.
    // Example:
    // if (bIsCharged)
    // {
    //     UE_LOG(LogTemp, Log, TEXT("Ultimate Tech Released!"));
    //     bIsCharged = false;
    //     EssenceCount = 0; // Reset essence or apply cost
    // }
    // else
    // {
    //     UE_LOG(LogTemp, Warning, TEXT("Attempted to release Ultimate Tech but not charged."));
    // }
}
