// ComboSystemTestManager.cpp
#include "ComboSystemTestManager.h"
#include "PRDComplianceValidator.h"
#include "CompleteComboMoveExample.h"
#include "ComboMoveData.h"

UComboSystemTestManager::UComboSystemTestManager()
{
    bSystemCompliant = false;
    LastTestResults = TEXT("No tests run yet");
}

bool UComboSystemTestManager::RunPRDComplianceTest()
{
    UE_LOG(LogTemp, Warning, TEXT("ComboSystemTestManager: Running PRD Compliance Test..."));
    
    bool bCompliant = FPRDComplianceValidator::ValidatePRDCompliance();
    FString Report = FPRDComplianceValidator::GenerateComplianceReport();
    
    UpdateTestResults(Report, bCompliant);
    
    if (GEngine)
    {
        FColor MessageColor = bCompliant ? FColor::Green : FColor::Orange;
        FString StatusMessage = bCompliant ? 
            TEXT("✓ PRD v0.33 Compliance: PASSED") : 
            TEXT("⚠ PRD v0.33 Compliance: REVIEW NEEDED");
        
        GEngine->AddOnScreenDebugMessage(-1, 10.0f, MessageColor, StatusMessage);
    }
    
    return bCompliant;
}

bool UComboSystemTestManager::RunIntegrationTests()
{
    UE_LOG(LogTemp, Warning, TEXT("ComboSystemTestManager: Running Integration Tests..."));
    
    bool bAllTestsPassed = true;
    FString TestReport = TEXT("=== Integration Test Results ===\n");
    
    // Test 1: Create and validate basic move
    UComboMoveData* BasicMove = NewObject<UComboMoveData>();
    if (BasicMove)
    {
        BasicMove->MoveID = FName("Integration_Test_Move");
        BasicMove->StartupFrames = 8;
        BasicMove->ActiveFrames = 4;
        BasicMove->RecoveryFrames = 12;
        BasicMove->DirectionMask = static_cast<int32>(EDirectionMask::Up);
        BasicMove->ResourceCost = 10;
        BasicMove->MinWeaponLevel = 2;
        
        bool bBasicMoveValid = ValidateComboMoveDataAsset(BasicMove);
        TestReport += FString::Printf(TEXT("Basic Move Creation: %s\n"), 
            bBasicMoveValid ? TEXT("PASS") : TEXT("FAIL"));
        bAllTestsPassed &= bBasicMoveValid;
    }
    else
    {
        TestReport += TEXT("Basic Move Creation: FAIL (Could not create object)\n");
        bAllTestsPassed = false;
    }
      // Test 2: Enhanced fields functionality
    UComboMoveData* EnhancedMove = NewObject<UComboMoveData>();
    if (EnhancedMove)
    {
        // Create test moves to reference
        UComboMoveData* TestMove1 = NewObject<UComboMoveData>();
        UComboMoveData* TestMove2 = NewObject<UComboMoveData>();
        TestMove1->MoveID = FName("Move1");
        TestMove2->MoveID = FName("Move2");        EnhancedMove->MoveID = FName("Enhanced_Test_Move");
        EnhancedMove->ComboWindowStart = 5;
        EnhancedMove->ComboWindowEnd = 10;
        
        // 使用新的辅助方法添加NextMoves
        EnhancedMove->ClearNextMoves();
        EnhancedMove->AddNextMove(TestMove1);
        EnhancedMove->AddNextMove(TestMove2);
        
        EnhancedMove->CancelCondition = static_cast<int32>(ECancelCondition::OnHit) | 
                                       static_cast<int32>(ECancelCondition::OnKill);
        EnhancedMove->PreserveBuffer = true;
        EnhancedMove->TargetPolicy = ETargetPolicy::Locked;
        EnhancedMove->FXTagArray = {FName("VFX_Test"), FName("SFX_Test")};
        
        bool bEnhancedValid = (EnhancedMove->ComboWindowStart == 5) &&
                             (EnhancedMove->ComboWindowEnd == 10) &&
                             (EnhancedMove->NextMoves.Num() == 2) &&
                             (EnhancedMove->NextMoves[0].IsValid()) &&
                             (EnhancedMove->NextMoves[1].IsValid()) &&
                             (EnhancedMove->NextMovesID.Num() == 2) &&
                             (EnhancedMove->NextMovesID[0] == FName("Move1")) &&
                             (EnhancedMove->NextMovesID[1] == FName("Move2")) &&
                             (EnhancedMove->CancelCondition != 0) &&
                             (EnhancedMove->PreserveBuffer == true) &&
                             (EnhancedMove->TargetPolicy == ETargetPolicy::Locked) &&
                             (EnhancedMove->FXTagArray.Num() == 2);
        
        TestReport += FString::Printf(TEXT("Enhanced Fields Test: %s\n"), 
            bEnhancedValid ? TEXT("PASS") : TEXT("FAIL"));
        bAllTestsPassed &= bEnhancedValid;
    }
    else
    {
        TestReport += TEXT("Enhanced Fields Test: FAIL (Could not create object)\n");
        bAllTestsPassed = false;
    }
    
    // Test 3: ChainRule structure
    UComboMoveData* ChainMove = NewObject<UComboMoveData>();
    if (ChainMove)
    {
        ChainMove->MoveID = FName("Chain_Test_Move");
        ChainMove->ChainRule.MaxLoop = 4;
        ChainMove->ChainRule.TriggerCondition = ETriggerCondition::OnHit;
        ChainMove->ChainRule.AutoAction = EAutoAction::Continue;
        ChainMove->ChainRule.ChainWindow = 0.15f;
        
        bool bChainValid = (ChainMove->ChainRule.MaxLoop == 4) &&
                          (ChainMove->ChainRule.TriggerCondition == ETriggerCondition::OnHit) &&
                          (ChainMove->ChainRule.AutoAction == EAutoAction::Continue) &&
                          (ChainMove->ChainRule.ChainWindow == 0.15f);
        
        TestReport += FString::Printf(TEXT("ChainRule Structure: %s\n"), 
            bChainValid ? TEXT("PASS") : TEXT("FAIL"));
        bAllTestsPassed &= bChainValid;
    }
    else
    {
        TestReport += TEXT("ChainRule Structure: FAIL (Could not create object)\n");
        bAllTestsPassed = false;
    }
    
    TestReport += FString::Printf(TEXT("Overall Integration: %s\n"), 
        bAllTestsPassed ? TEXT("PASS") : TEXT("FAIL"));
    
    UpdateTestResults(TestReport, bAllTestsPassed);
    
    if (GEngine)
    {
        FColor MessageColor = bAllTestsPassed ? FColor::Green : FColor::Red;
        GEngine->AddOnScreenDebugMessage(-1, 8.0f, MessageColor, 
            FString::Printf(TEXT("Integration Tests: %s"), 
                bAllTestsPassed ? TEXT("PASSED") : TEXT("FAILED")));
    }
    
    return bAllTestsPassed;
}

bool UComboSystemTestManager::RunPerformanceTests()
{
    UE_LOG(LogTemp, Warning, TEXT("ComboSystemTestManager: Running Performance Tests..."));
    
    const int32 TestIterations = 500;
    double StartTime = FPlatformTime::Seconds();
    
    // Create multiple ComboMoveData objects to test performance
    TArray<UComboMoveData*> TestMoves;
    TestMoves.Reserve(TestIterations);
    
    for (int32 i = 0; i < TestIterations; i++)
    {
        UComboMoveData* TestMove = NewObject<UComboMoveData>();
        if (TestMove)
        {
            // Configure with all enhanced fields to simulate real usage
            TestMove->MoveID = FName(*FString::Printf(TEXT("PerfTest_%d"), i));
            TestMove->DirectionMask = static_cast<int32>(EDirectionMask::Up) | static_cast<int32>(EDirectionMask::Right);
            TestMove->ComboWindowStart = FMath::RandRange(3, 8);
            TestMove->ComboWindowEnd = FMath::RandRange(10, 20);
            //TestMove->NextMoves = {FName("Move1"), FName("Move2")};
            TestMove->CancelCondition = static_cast<int32>(ECancelCondition::OnHit);
            TestMove->PreserveBuffer = (i % 2 == 0);
            TestMove->TargetPolicy = static_cast<ETargetPolicy>(i % 3);
            TestMove->ResourceCost = FMath::RandRange(5, 25);
            TestMove->MinWeaponLevel = FMath::RandRange(1, 5);
            TestMove->FXTagArray = {FName("VFX_Test"), FName("SFX_Test")};
            
            TestMoves.Add(TestMove);
        }
    }
    
    double EndTime = FPlatformTime::Seconds();
    double ElapsedTime = EndTime - StartTime;
    
    bool bPerformanceAcceptable = (ElapsedTime < 0.5) && (TestMoves.Num() == TestIterations);
    
    FString PerformanceReport = FString::Printf(
        TEXT("=== Performance Test Results ===\n")
        TEXT("Test Iterations: %d\n")
        TEXT("Elapsed Time: %.3f seconds\n")
        TEXT("Average Time per Move: %.6f seconds\n")
        TEXT("Performance Status: %s\n"),
        TestIterations,
        ElapsedTime,
        ElapsedTime / TestIterations,
        bPerformanceAcceptable ? TEXT("ACCEPTABLE") : TEXT("NEEDS OPTIMIZATION")
    );
    
    UpdateTestResults(PerformanceReport, bPerformanceAcceptable);
    
    if (GEngine)
    {
        FColor MessageColor = bPerformanceAcceptable ? FColor::Green : FColor::Yellow;
        GEngine->AddOnScreenDebugMessage(-1, 8.0f, MessageColor, 
            FString::Printf(TEXT("Performance: %.3fs for %d moves"), ElapsedTime, TestIterations));
    }
    
    UE_LOG(LogTemp, Log, TEXT("Performance Test: Created %d moves in %.3f seconds"), 
        TestMoves.Num(), ElapsedTime);
    
    return bPerformanceAcceptable;
}

void UComboSystemTestManager::CreateExampleComboMoves()
{
    UE_LOG(LogTemp, Warning, TEXT("ComboSystemTestManager: Creating Example Combo Moves..."));
    
    UCompleteComboMoveExample* ExampleCreator = NewObject<UCompleteComboMoveExample>();
    if (ExampleCreator)
    {
        try
        {
            ExampleCreator->CreateFlyingSwallowExample();
            ExampleCreator->CreateIzunaGrabExample();
            ExampleCreator->CreateBasicLightAttackExample();
            ExampleCreator->CreateChainAttackExample();
            
            FString SuccessMessage = TEXT("✓ All example combo moves created successfully");
            UpdateTestResults(SuccessMessage, true);
            
            if (GEngine)
            {
                GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, 
                    TEXT("✓ Example moves created"));
            }
            
            UE_LOG(LogTemp, Log, TEXT("Successfully created all example combo moves"));
        }
        catch (...)
        {
            FString ErrorMessage = TEXT("✗ Failed to create example combo moves");
            UpdateTestResults(ErrorMessage, false);
            
            if (GEngine)
            {
                GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, 
                    TEXT("✗ Failed to create examples"));
            }
            
            UE_LOG(LogTemp, Error, TEXT("Failed to create example combo moves"));
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to create ExampleCreator object"));
    }
}

void UComboSystemTestManager::LogSystemStatus()
{
    UE_LOG(LogTemp, Warning, TEXT("=== Combo System Status Report ==="));
    UE_LOG(LogTemp, Warning, TEXT("PRD Version: v0.33"));
    UE_LOG(LogTemp, Warning, TEXT("Implementation Status: Enhanced ComboMoveData"));
    UE_LOG(LogTemp, Warning, TEXT("Last Test Results: %s"), *LastTestResults);
    UE_LOG(LogTemp, Warning, TEXT("System Compliant: %s"), bSystemCompliant ? TEXT("Yes") : TEXT("No"));
    UE_LOG(LogTemp, Warning, TEXT("====================================="));
    
    if (GEngine)
    {
        FColor StatusColor = bSystemCompliant ? FColor::Green : FColor::Orange;
        GEngine->AddOnScreenDebugMessage(-1, 8.0f, StatusColor, 
            FString::Printf(TEXT("Combo System Status: %s"), 
                bSystemCompliant ? TEXT("COMPLIANT") : TEXT("NEEDS REVIEW")));
    }
}

bool UComboSystemTestManager::ValidateComboMoveDataAsset(UComboMoveData* MoveData)
{
    if (!MoveData)
    {
        UE_LOG(LogTemp, Error, TEXT("ValidateComboMoveDataAsset: MoveData is null"));
        return false;
    }
    
    // Basic validation
    if (MoveData->StartupFrames < 0 || MoveData->ActiveFrames < 0 || MoveData->RecoveryFrames < 0)
    {
        UE_LOG(LogTemp, Error, TEXT("ValidateComboMoveDataAsset: Invalid frame data"));
        return false;
    }
    
    if (MoveData->ComboWindowStart > MoveData->ComboWindowEnd)
    {
        UE_LOG(LogTemp, Error, TEXT("ValidateComboMoveDataAsset: Invalid combo window"));
        return false;
    }
    
    if (MoveData->ResourceCost < 0)
    {
        UE_LOG(LogTemp, Error, TEXT("ValidateComboMoveDataAsset: Invalid resource cost"));
        return false;
    }
    
    if (MoveData->MinWeaponLevel < 1)
    {
        UE_LOG(LogTemp, Error, TEXT("ValidateComboMoveDataAsset: Invalid weapon level"));
        return false;
    }
    
    return true;
}

FString UComboSystemTestManager::GetMoveDataDebugInfo(UComboMoveData* MoveData)
{
    if (!MoveData)
    {
        return TEXT("MoveData is null");
    }
    
    FString DebugInfo = FString::Printf(TEXT("=== %s Debug Info ===\n"), *MoveData->MoveID.ToString());
    DebugInfo += FString::Printf(TEXT("Frames: %d/%d/%d (S/A/R)\n"), 
        MoveData->StartupFrames, MoveData->ActiveFrames, MoveData->RecoveryFrames);
    DebugInfo += FString::Printf(TEXT("Combo Window: %d-%d\n"), 
        MoveData->ComboWindowStart, MoveData->ComboWindowEnd);
    DebugInfo += FString::Printf(TEXT("Direction Mask: %d\n"), MoveData->DirectionMask);
    DebugInfo += FString::Printf(TEXT("Next Moves: %d\n"), MoveData->NextMoves.Num());
    DebugInfo += FString::Printf(TEXT("Cancel Condition: %d\n"), MoveData->CancelCondition);
    DebugInfo += FString::Printf(TEXT("Resource Cost: %d\n"), MoveData->ResourceCost);
    DebugInfo += FString::Printf(TEXT("Min Weapon Level: %d\n"), MoveData->MinWeaponLevel);
    DebugInfo += FString::Printf(TEXT("FX Tags: %d\n"), MoveData->FXTagArray.Num());
    DebugInfo += FString::Printf(TEXT("Preserve Buffer: %s\n"), 
        MoveData->PreserveBuffer ? TEXT("Yes") : TEXT("No"));
    DebugInfo += FString::Printf(TEXT("Target Policy: %d\n"), static_cast<int32>(MoveData->TargetPolicy));
    
    return DebugInfo;
}

void UComboSystemTestManager::UpdateTestResults(const FString& Results, bool bCompliant)
{
    LastTestResults = Results;
    bSystemCompliant = bCompliant;
    
    UE_LOG(LogTemp, Log, TEXT("Test results updated. Compliant: %s"), 
        bCompliant ? TEXT("Yes") : TEXT("No"));
}
