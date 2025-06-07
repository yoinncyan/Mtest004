// ComboSystemTestManager.h
#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Engine/Engine.h"
#include "ComboSystemTestManager.generated.h"

/**
 * Test manager class for ComboMoveData system
 * Provides Blueprint-callable functions for testing PRD compliance
 */
UCLASS(BlueprintType, Blueprintable)
class MTEST004_API UComboSystemTestManager : public UObject
{
    GENERATED_BODY()

public:
    UComboSystemTestManager();    // Blueprint-callable test functions
    UFUNCTION(BlueprintCallable, Category = "Combo System Tests", meta = (CallInEditor = "true"))
    bool RunPRDComplianceTest();

    UFUNCTION(BlueprintCallable, Category = "Combo System Tests", meta = (CallInEditor = "true"))
    bool RunIntegrationTests();

    UFUNCTION(BlueprintCallable, Category = "Combo System Tests", meta = (CallInEditor = "true"))
    bool RunPerformanceTests();

    UFUNCTION(BlueprintCallable, Category = "Combo System Tests", meta = (CallInEditor = "true"))
    void CreateExampleComboMoves();

    UFUNCTION(BlueprintCallable, Category = "Combo System Tests", meta = (CallInEditor = "true"))
    void LogSystemStatus();

    // Get test results
    UFUNCTION(BlueprintPure, Category = "Combo System Tests")
    FString GetLastTestResults() const { return LastTestResults; }

    UFUNCTION(BlueprintPure, Category = "Combo System Tests")
    bool IsSystemFullyCompliant() const { return bSystemCompliant; }

    // Validation utilities
    UFUNCTION(BlueprintCallable, Category = "Combo System Tests")
    bool ValidateComboMoveDataAsset(class UComboMoveData* MoveData);

    UFUNCTION(BlueprintCallable, Category = "Combo System Tests")
    FString GetMoveDataDebugInfo(class UComboMoveData* MoveData);

private:
    UPROPERTY()
    FString LastTestResults;

    UPROPERTY()
    bool bSystemCompliant;

    void UpdateTestResults(const FString& Results, bool bCompliant);
};
