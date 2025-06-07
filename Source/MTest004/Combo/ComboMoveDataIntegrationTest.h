// ComboMoveDataIntegrationTest.h
#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "ComboMoveData.h"
#include "CompleteComboMoveExample.h"
#include "ComboMoveDataIntegrationTest.generated.h"

/**
 * Integration test class for verifying all PRD-specified ComboMoveData fields
 * Tests the complete enhanced ComboMoveData system according to PRD v0.33
 */
UCLASS(BlueprintType, Category = "Combat Tests")
class MTEST004_API UComboMoveDataIntegrationTest : public UObject
{
    GENERATED_BODY()

public:
    UComboMoveDataIntegrationTest();

    // Main test functions
    UFUNCTION(BlueprintCallable, Category = "Testing")
    bool RunAllTests();

    UFUNCTION(BlueprintCallable, Category = "Testing")
    bool TestBasicFieldsIntegrity();

    UFUNCTION(BlueprintCallable, Category = "Testing")
    bool TestEnhancedFieldsIntegrity();

    UFUNCTION(BlueprintCallable, Category = "Testing")
    bool TestChainRuleStructure();

    UFUNCTION(BlueprintCallable, Category = "Testing")
    bool TestEnumDefinitions();

    UFUNCTION(BlueprintCallable, Category = "Testing")
    bool TestGameplayTagIntegration();

    UFUNCTION(BlueprintCallable, Category = "Testing")
    bool TestValidationSystem();

    UFUNCTION(BlueprintCallable, Category = "Testing")
    bool TestExampleImplementations();

    // Performance test for the enhanced data structure
    UFUNCTION(BlueprintCallable, Category = "Testing")
    bool PerformanceTestEnhancedStructure();

    // Blueprint integration test
    UFUNCTION(BlueprintCallable, Category = "Testing")
    bool TestBlueprintVisibility();

    // Generate comprehensive test report
    UFUNCTION(BlueprintCallable, Category = "Testing")
    FString GenerateTestReport();

private:
    // Helper functions
    UComboMoveData* CreateTestMoveData(const FName& MoveID);
    bool ValidateFieldDefaults(const UComboMoveData* MoveData);
    bool TestDirectionMaskOperations();
    bool TestCancelConditionBitflags();
    bool TestTargetPolicyEnum();
    bool TestChainRuleConfiguration();
    
    // Test data
    UPROPERTY()
    TObjectPtr<UCompleteComboMoveExample> ExampleCreator;
    
    UPROPERTY()
    TArray<TObjectPtr<UComboMoveData>> TestMoves;
    
    // Test results tracking
    TArray<FString> TestResults;
    int32 PassedTests;
    int32 TotalTests;
};
