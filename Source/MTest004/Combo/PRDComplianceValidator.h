// PRDComplianceValidator.h
#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"

/**
 * Static validator class to verify PRD v0.33 compliance
 * Validates that all required ComboMoveData fields are present and functional
 */
class MTEST004_API FPRDComplianceValidator
{
public:
    // Main validation function
    static bool ValidatePRDCompliance();
    
    // Individual field validation
    static bool ValidateDirectionMaskField();
    static bool ValidateComboWindowFields();
    static bool ValidateNextMovesField();
    static bool ValidateCancelConditionField();
    static bool ValidateChainRuleStructure();
    static bool ValidatePreserveBufferField();
    static bool ValidateTargetPolicyField();
    static bool ValidateResourceCostField();
    static bool ValidateDamageScalingField();
    static bool ValidateGameplayTagsField();
    static bool ValidateMinWeaponLevelField();
    static bool ValidateFXTagArrayField();
    
    // Generate compliance report
    static FString GenerateComplianceReport();
    
private:
    static TArray<FString> ValidationResults;
    static int32 PassedValidations;
    static int32 TotalValidations;
    
    static void AddValidationResult(const FString& TestName, bool bPassed, const FString& Details = TEXT(""));
    static void LogValidationResult(const FString& TestName, bool bPassed);
};
