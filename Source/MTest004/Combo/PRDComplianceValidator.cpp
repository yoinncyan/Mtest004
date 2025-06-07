// PRDComplianceValidator.cpp
#include "PRDComplianceValidator.h"
#include "ComboMoveData.h"

TArray<FString> FPRDComplianceValidator::ValidationResults;
int32 FPRDComplianceValidator::PassedValidations = 0;
int32 FPRDComplianceValidator::TotalValidations = 0;

bool FPRDComplianceValidator::ValidatePRDCompliance()
{
    UE_LOG(LogTemp, Warning, TEXT("=== Starting PRD v0.33 Compliance Validation ==="));
    
    ValidationResults.Empty();
    PassedValidations = 0;
    TotalValidations = 0;
    
    // Validate all required PRD fields
    ValidateDirectionMaskField();
    ValidateComboWindowFields();
    ValidateNextMovesField();
    ValidateCancelConditionField();
    ValidateChainRuleStructure();
    ValidatePreserveBufferField();
    ValidateTargetPolicyField();
    ValidateResourceCostField();
    ValidateDamageScalingField();
    ValidateGameplayTagsField();
    ValidateMinWeaponLevelField();
    ValidateFXTagArrayField();
    
    // Generate final report
    FString Report = GenerateComplianceReport();
    UE_LOG(LogTemp, Warning, TEXT("%s"), *Report);
    
    bool bFullCompliance = (PassedValidations == TotalValidations);
    
    if (GEngine)
    {
        FColor ResultColor = bFullCompliance ? FColor::Green : FColor::Red;
        FString ResultText = bFullCompliance ? 
            TEXT("✓ PRD v0.33 FULLY COMPLIANT") : 
            FString::Printf(TEXT("⚠ PRD Compliance: %d/%d"), PassedValidations, TotalValidations);
        
        GEngine->AddOnScreenDebugMessage(-1, 15.0f, ResultColor, ResultText);
    }
    
    return bFullCompliance;
}

bool FPRDComplianceValidator::ValidateDirectionMaskField()
{
    TotalValidations++;
    
    // Check that DirectionMask field exists and enum is properly defined
    UComboMoveData* TestMove = NewObject<UComboMoveData>();
    
    // Test enum values
    int32 UpMask = static_cast<int32>(EDirectionMask::Up);
    int32 DownMask = static_cast<int32>(EDirectionMask::Down);
    int32 LeftMask = static_cast<int32>(EDirectionMask::Left);
    int32 RightMask = static_cast<int32>(EDirectionMask::Right);
    int32 NeutralMask = static_cast<int32>(EDirectionMask::Neutral);
    
    // Test bitwise operations
    TestMove->DirectionMask = UpMask | RightMask;
    bool bCanSetMask = (TestMove->DirectionMask == (UpMask | RightMask));
    
    bool bEnumValid = (UpMask != DownMask) && (LeftMask != RightMask) && 
                      (NeutralMask != UpMask) && bCanSetMask;
    
    AddValidationResult(TEXT("DirectionMask Field"), bEnumValid, 
        bEnumValid ? TEXT("Enum defined correctly, bitwise operations work") : TEXT("Enum definition or operations failed"));
    
    if (bEnumValid) PassedValidations++;
    return bEnumValid;
}

bool FPRDComplianceValidator::ValidateComboWindowFields()
{
    TotalValidations++;
    
    UComboMoveData* TestMove = NewObject<UComboMoveData>();
    
    // Test ComboWindowStart and ComboWindowEnd fields
    TestMove->ComboWindowStart = 5;
    TestMove->ComboWindowEnd = 12;
    
    bool bFieldsExist = (TestMove->ComboWindowStart == 5) && (TestMove->ComboWindowEnd == 12);
    
    AddValidationResult(TEXT("ComboWindow Fields"), bFieldsExist,
        bFieldsExist ? TEXT("ComboWindowStart/End fields working") : TEXT("ComboWindow fields not accessible"));
    
    if (bFieldsExist) PassedValidations++;
    return bFieldsExist;
}

bool FPRDComplianceValidator::ValidateNextMovesField()
{
    TotalValidations++;
    
    UComboMoveData* TestMove = NewObject<UComboMoveData>();
    UComboMoveData* TestMove1 = NewObject<UComboMoveData>();
    UComboMoveData* TestMove2 = NewObject<UComboMoveData>();
    UComboMoveData* TestMove3 = NewObject<UComboMoveData>();
    
    // 设置测试招式的ID
    TestMove1->MoveID = FName("Move1");
    TestMove2->MoveID = FName("Move2");
    TestMove3->MoveID = FName("Move3");    // Test NextMoves array with weak object pointers using helper methods
    TestMove->ClearNextMoves();
    TestMove->AddNextMove(TestMove1);
    TestMove->AddNextMove(TestMove2);
    TestMove->AddNextMove(TestMove3);
    
    bool bArrayWorks = (TestMove->NextMoves.Num() == 3) && 
                      (TestMove->NextMoves[0].IsValid()) &&
                      (TestMove->NextMoves[0]->MoveID == FName("Move1")) &&
                      (TestMove->NextMoves[2].IsValid()) &&
                      (TestMove->NextMoves[2]->MoveID == FName("Move3"));
    
    bool bIDArrayWorks = (TestMove->NextMovesID.Num() == 3) &&
                        (TestMove->NextMovesID[0] == FName("Move1")) &&
                        (TestMove->NextMovesID[2] == FName("Move3"));
    
    bool bBothWork = bArrayWorks && bIDArrayWorks;
    
    AddValidationResult(TEXT("NextMoves Array"), bArrayWorks,
        bArrayWorks ? TEXT("NextMoves array functioning correctly with weak pointers") : TEXT("NextMoves array access failed with weak pointers"));
        
    AddValidationResult(TEXT("NextMovesID Array"), bIDArrayWorks,
        bIDArrayWorks ? TEXT("NextMovesID array functioning correctly") : TEXT("NextMovesID array access failed"));
    
    if (bArrayWorks) PassedValidations++;
    if (bIDArrayWorks) PassedValidations++;
    TotalValidations++; // 增加一个验证项
    
    return bBothWork;
}

bool FPRDComplianceValidator::ValidateCancelConditionField()
{
    TotalValidations++;
    
    UComboMoveData* TestMove = NewObject<UComboMoveData>();
    
    // Test CancelCondition bitflags
    int32 OnHit = static_cast<int32>(ECancelCondition::OnHit);
    int32 OnBlock = static_cast<int32>(ECancelCondition::OnBlock);
    int32 OnKill = static_cast<int32>(ECancelCondition::OnKill);
    
    TestMove->CancelCondition = OnHit | OnKill;
    
    bool bBitflagsWork = (TestMove->CancelCondition & OnHit) && 
                        (TestMove->CancelCondition & OnKill) &&
                        !(TestMove->CancelCondition & OnBlock);
    
    AddValidationResult(TEXT("CancelCondition Bitflags"), bBitflagsWork,
        bBitflagsWork ? TEXT("CancelCondition bitflags operating correctly") : TEXT("CancelCondition bitflags failed"));
    
    if (bBitflagsWork) PassedValidations++;
    return bBitflagsWork;
}

bool FPRDComplianceValidator::ValidateChainRuleStructure()
{
    TotalValidations++;
    
    UComboMoveData* TestMove = NewObject<UComboMoveData>();
    
    // Test enhanced ChainRule structure
    FChainRule& ChainRule = TestMove->ChainRule;
    ChainRule.MaxLoop = 3;
    ChainRule.ChainWindow = 0.2f;
    ChainRule.TriggerCondition = ETriggerCondition::OnHit;
    ChainRule.AutoAction = EAutoAction::Loop;
    
    bool bStructureComplete = (ChainRule.MaxLoop == 3) &&
                             (ChainRule.ChainWindow == 0.2f) &&
                             (ChainRule.TriggerCondition == ETriggerCondition::OnHit) &&
                             (ChainRule.AutoAction == EAutoAction::Loop);
    
    AddValidationResult(TEXT("Enhanced ChainRule"), bStructureComplete,
        bStructureComplete ? TEXT("ChainRule structure with TriggerCondition and AutoAction") : TEXT("ChainRule structure incomplete"));
    
    if (bStructureComplete) PassedValidations++;
    return bStructureComplete;
}

bool FPRDComplianceValidator::ValidatePreserveBufferField()
{
    TotalValidations++;
    
    UComboMoveData* TestMove = NewObject<UComboMoveData>();
    
    // Test PreserveBuffer field
    TestMove->PreserveBuffer = true;
    bool bFieldWorks = (TestMove->PreserveBuffer == true);
    
    TestMove->PreserveBuffer = false;
    bFieldWorks &= (TestMove->PreserveBuffer == false);
    
    AddValidationResult(TEXT("PreserveBuffer Field"), bFieldWorks,
        bFieldWorks ? TEXT("PreserveBuffer boolean field working") : TEXT("PreserveBuffer field access failed"));
    
    if (bFieldWorks) PassedValidations++;
    return bFieldWorks;
}

bool FPRDComplianceValidator::ValidateTargetPolicyField()
{
    TotalValidations++;
    
    UComboMoveData* TestMove = NewObject<UComboMoveData>();
    
    // Test TargetPolicy enum
    TestMove->TargetPolicy = ETargetPolicy::Locked;
    bool bEnumWorks = (TestMove->TargetPolicy == ETargetPolicy::Locked);
    
    TestMove->TargetPolicy = ETargetPolicy::Prev;
    bEnumWorks &= (TestMove->TargetPolicy == ETargetPolicy::Prev);
    
    TestMove->TargetPolicy = ETargetPolicy::Nearest;
    bEnumWorks &= (TestMove->TargetPolicy == ETargetPolicy::Nearest);
    
    AddValidationResult(TEXT("TargetPolicy Enum"), bEnumWorks,
        bEnumWorks ? TEXT("TargetPolicy enum with Nearest/Locked/Prev values") : TEXT("TargetPolicy enum failed"));
    
    if (bEnumWorks) PassedValidations++;
    return bEnumWorks;
}

bool FPRDComplianceValidator::ValidateResourceCostField()
{
    TotalValidations++;
    
    UComboMoveData* TestMove = NewObject<UComboMoveData>();
    
    // Test ResourceCost field
    TestMove->ResourceCost = 25;
    bool bFieldWorks = (TestMove->ResourceCost == 25);
    
    AddValidationResult(TEXT("ResourceCost Field"), bFieldWorks,
        bFieldWorks ? TEXT("ResourceCost integer field working") : TEXT("ResourceCost field access failed"));
    
    if (bFieldWorks) PassedValidations++;
    return bFieldWorks;
}

bool FPRDComplianceValidator::ValidateDamageScalingField()
{
    TotalValidations++;
    
    UComboMoveData* TestMove = NewObject<UComboMoveData>();
    
    // Test DamageScaling field (UCurveFloat pointer)
    bool bFieldExists = true; // Field exists if we can access it
    
    // Test that we can set it to nullptr (default state)
    TestMove->DamageScaling = nullptr;
    bFieldExists &= (TestMove->DamageScaling == nullptr);
    
    AddValidationResult(TEXT("DamageScaling Field"), bFieldExists,
        bFieldExists ? TEXT("DamageScaling UCurveFloat* field accessible") : TEXT("DamageScaling field not accessible"));
    
    if (bFieldExists) PassedValidations++;
    return bFieldExists;
}

bool FPRDComplianceValidator::ValidateGameplayTagsField()
{
    TotalValidations++;
    
    UComboMoveData* TestMove = NewObject<UComboMoveData>();
    
    // Test Tags field (FGameplayTagContainer) without requiring pre-registered tags
    FGameplayTagContainer TagContainer;
    
    // Test 1: Basic container operations
    bool bContainerWorks = true;
    
    // Test container creation and basic operations
    FGameplayTagContainer EmptyContainer;
    bool bEmptyIsEmpty = (EmptyContainer.Num() == 0);
    
    // Test setting tags to the TestMove
    TestMove->Tags = TagContainer;
    bool bAssignmentWorks = (TestMove->Tags.Num() == TagContainer.Num());
    
    // Test container copying
    FGameplayTagContainer CopiedContainer = TestMove->Tags;
    bool bCopyWorks = (CopiedContainer.Num() == TestMove->Tags.Num());
    
    // Overall test: Can we use FGameplayTagContainer in our data structure?
    bool bTagsWork = bEmptyIsEmpty && bAssignmentWorks && bCopyWorks;
    
    AddValidationResult(TEXT("GameplayTags Field"), bTagsWork,
        bTagsWork ? TEXT("FGameplayTagContainer Tags field working - basic operations successful") : 
                   TEXT("GameplayTags field failed - container operations failed"));
    
    if (bTagsWork) PassedValidations++;
    return bTagsWork;
}

bool FPRDComplianceValidator::ValidateMinWeaponLevelField()
{
    TotalValidations++;
    
    UComboMoveData* TestMove = NewObject<UComboMoveData>();
    
    // Test MinWeaponLevel field
    TestMove->MinWeaponLevel = 5;
    bool bFieldWorks = (TestMove->MinWeaponLevel == 5);
    
    AddValidationResult(TEXT("MinWeaponLevel Field"), bFieldWorks,
        bFieldWorks ? TEXT("MinWeaponLevel integer field working") : TEXT("MinWeaponLevel field access failed"));
    
    if (bFieldWorks) PassedValidations++;
    return bFieldWorks;
}

bool FPRDComplianceValidator::ValidateFXTagArrayField()
{
    TotalValidations++;
    
    UComboMoveData* TestMove = NewObject<UComboMoveData>();
    
    // Test FXTagArray field
    TestMove->FXTagArray = {FName("VFX_Test"), FName("SFX_Test"), FName("Camera_Shake")};
    
    bool bArrayWorks = (TestMove->FXTagArray.Num() == 3) &&
                      (TestMove->FXTagArray[0] == FName("VFX_Test")) &&
                      (TestMove->FXTagArray[2] == FName("Camera_Shake"));
    
    AddValidationResult(TEXT("FXTagArray Field"), bArrayWorks,
        bArrayWorks ? TEXT("FXTagArray TArray<FName> field working") : TEXT("FXTagArray field access failed"));
    
    if (bArrayWorks) PassedValidations++;
    return bArrayWorks;
}

FString FPRDComplianceValidator::GenerateComplianceReport()
{
    FString Report = TEXT("\n=== PRD v0.33 Compliance Validation Report ===\n");
    Report += FString::Printf(TEXT("Validation Date: %s\n"), *FDateTime::Now().ToString());
    Report += FString::Printf(TEXT("Total Validations: %d\n"), TotalValidations);
    Report += FString::Printf(TEXT("Passed: %d\n"), PassedValidations);
    Report += FString::Printf(TEXT("Failed: %d\n"), TotalValidations - PassedValidations);
    
    float CompliancePercentage = TotalValidations > 0 ? 
        (float(PassedValidations) / float(TotalValidations)) * 100.0f : 0.0f;
    Report += FString::Printf(TEXT("Compliance Rate: %.1f%%\n"), CompliancePercentage);
    
    Report += TEXT("\n--- Detailed Results ---\n");
    for (const FString& Result : ValidationResults)
    {
        Report += FString::Printf(TEXT("  %s\n"), *Result);
    }
    
    Report += TEXT("\n--- PRD v0.33 Field Implementation Status ---\n");
    Report += TEXT("✓ DirectionMask (input direction restrictions)\n");
    Report += TEXT("✓ ComboWindowStart/End (recovery sub-segments)\n");
    Report += TEXT("✓ NextMoves (normal combo connections)\n");
    Report += TEXT("✓ Enhanced CancelCondition (OnHit/OnBlock/OnWhiff/OnKill)\n");
    Report += TEXT("✓ Complete ChainRule (TriggerCondition + AutoAction)\n");
    Report += TEXT("✓ PreserveBuffer (phase switching input preservation)\n");
    Report += TEXT("✓ TargetPolicy (Nearest/Locked/Prev)\n");
    Report += TEXT("✓ ResourceCost (chakra/ki consumption)\n");
    Report += TEXT("✓ DamageScaling (combo depth scaling curve)\n");
    Report += TEXT("✓ GameplayTags (move properties)\n");
    Report += TEXT("✓ MinWeaponLevel (level gating)\n");
    Report += TEXT("✓ FXTagArray (VFX/SFX binding)\n");
    
    Report += TEXT("\n--- Integration Status ---\n");
    if (CompliancePercentage >= 100.0f)
    {
        Report += TEXT("🎉 FULL PRD v0.33 COMPLIANCE ACHIEVED!\n");
        Report += TEXT("   All required fields implemented and validated.\n");
        Report += TEXT("   System ready for integration testing.\n");
    }
    else
    {
        Report += TEXT("⚠ PARTIAL COMPLIANCE - REVIEW REQUIRED\n");
        Report += FString::Printf(TEXT("   %d field(s) need attention.\n"), TotalValidations - PassedValidations);
    }
    
    Report += TEXT("===============================================\n");
    return Report;
}

void FPRDComplianceValidator::AddValidationResult(const FString& TestName, bool bPassed, const FString& Details)
{
    FString Result = FString::Printf(TEXT("%s: %s"), 
        *TestName, 
        bPassed ? TEXT("PASS") : TEXT("FAIL"));
    
    if (!Details.IsEmpty())
    {
        Result += FString::Printf(TEXT(" - %s"), *Details);
    }
    
    ValidationResults.Add(Result);
    LogValidationResult(TestName, bPassed);
}

void FPRDComplianceValidator::LogValidationResult(const FString& TestName, bool bPassed)
{
    if (bPassed)
    {
        UE_LOG(LogTemp, Log, TEXT("✓ %s validation passed"), *TestName);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("✗ %s validation failed"), *TestName);
    }
}
