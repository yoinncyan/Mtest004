// AdvancedSystems.h
#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "AdvancedSystems.generated.h"

// 本质爆发技系统
UCLASS(ClassGroup=(Combat), meta=(BlueprintSpawnableComponent))
class MTEST004_API UObliterationSystem : public UObject
{
    GENERATED_BODY()
public:
    void CheckOTConditions(AActor* Target);
    
private:
    UPROPERTY()
    TMap<FName, TSubclassOf<UAnimInstance>> OTVariants;
};

// 终极技系统
UCLASS(ClassGroup=(Combat), meta=(BlueprintSpawnableComponent))
class MTEST004_API UUltimateTechSystem : public UObject
{
    GENERATED_BODY()
public:
    void StartCharging();
    void ReleaseUT();
    
private:
    void OnChargeComplete(); // Added declaration

    UPROPERTY()
    float ChargeStartTime = 0.0f;
    
    UPROPERTY()
    int32 EssenceCount = 0; // Initialized
    
    FTimerHandle ChargeTimerHandle; // Added declaration
};
