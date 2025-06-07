// CompleteComboMoveExample.h
#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "ComboMoveData.h"
#include "CompleteComboMoveExample.generated.h"

/**
 * Example class showing complete usage of all ComboMoveData fields
 * according to PRD specifications with detailed examples
 */
UCLASS(BlueprintType, Category = "Combat Examples")
class MTEST004_API UCompleteComboMoveExample : public UObject
{
    GENERATED_BODY()

public:
    UCompleteComboMoveExample();    // Example DataAsset creation functions
    UFUNCTION(BlueprintCallable, Category = "Examples", meta = (CallInEditor = "true"))
    void CreateFlyingSwallowExample();

    UFUNCTION(BlueprintCallable, Category = "Examples", meta = (CallInEditor = "true"))
    void CreateIzunaGrabExample();

    UFUNCTION(BlueprintCallable, Category = "Examples", meta = (CallInEditor = "true"))
    void CreateBasicLightAttackExample();

    UFUNCTION(BlueprintCallable, Category = "Examples", meta = (CallInEditor = "true"))
    void CreateChainAttackExample();

    // Validation and testing functions
    UFUNCTION(BlueprintCallable, Category = "Validation")
    bool ValidateComboMoveData(const UComboMoveData* MoveData) const;

    UFUNCTION(BlueprintCallable, Category = "Debug")
    FString GetMoveDataDebugInfo(const UComboMoveData* MoveData) const;

private:
    // Helper functions for setting up common configurations
    void SetupBasicMoveProperties(UComboMoveData* MoveData, const FName& ID,
        int32 StartupF, int32 ActiveF, int32 RecoveryF);

    void SetupChainRule(FChainRule& ChainRule, int32 MaxLoops,
        ETriggerCondition Trigger, float WindowSeconds);

    void SetupCancelConditions(UComboMoveData* MoveData,
        TArray<ECancelCondition> Conditions);

    void SetupDirectionMask(UComboMoveData* MoveData,
        TArray<EDirectionMask> AllowedDirections);
};