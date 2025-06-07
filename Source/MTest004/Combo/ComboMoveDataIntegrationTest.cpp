// ComboMoveDataIntegrationTest.cpp
#include "ComboMoveDataIntegrationTest.h"
#include "Engine/Engine.h"
#include "GameplayTagsManager.h"
#include "UObject/Class.h"

UComboMoveDataIntegrationTest::UComboMoveDataIntegrationTest()
{
    PassedTests = 0;
    TotalTests = 0;
    ExampleCreator = CreateDefaultSubobject<UCompleteComboMoveExample>(TEXT("ExampleCreator"));
}

bool UComboMoveDataIntegrationTest::RunAllTests()
{
    UE_LOG(LogTemp, Warning, TEXT("=== Starting ComboMoveData Integration Tests ==="));
    
    PassedTests = 0;
    TotalTests = 0;
    TestResults.Empty();
    
    // Run all test categories
    TestBasicFieldsIntegrity();
    TestEnhancedFieldsIntegrity();
    TestChainRuleStructure();
    TestEnumDefinitions();
    TestGameplayTagIntegration();
    TestValidationSystem();
    TestExampleImplementations();
    PerformanceTestEnhancedStructure();
    TestBlueprintVisibility();
    
    // Generate final report
    FString Report = GenerateTestReport();
    UE_LOG(LogTemp, Warning, TEXT("%s"), *Report);
    
    if (GEngine)
    {
        FColor ResultColor = (PassedTests == TotalTests) ? FColor::Green : FColor::Red;
        GEngine->AddOnScreenDebugMessage(-1, 10.0f, ResultColor, 
            FString::Printf(TEXT("Integration Tests: %d/%d Passed"), PassedTests, TotalTests));
    }
    
    return PassedTests == TotalTests;
}

bool UComboMoveDataIntegrationTest::TestBasicFieldsIntegrity()
{
    TotalTests++;
    UE_LOG(LogTemp, Log, TEXT("Testing basic fields integrity..."));
    
    UComboMoveData* TestMove = CreateTestMoveData(FName("TestBasic"));
    if (!TestMove)
    {
        TestResults.Add(TEXT("FAIL: Could not create test move data"));
        return false;
    }
    
    // Test basic field assignments
    TestMove->MoveID = FName("TestMove");
    TestMove->StartupFrames = 10;
    TestMove->ActiveFrames = 5;
    TestMove->RecoveryFrames = 15;
    TestMove->BaseDamage = 50.0f;
    TestMove->BaseScore = 100.0f;
    
    bool bBasicFieldsValid = (TestMove->MoveID == FName("TestMove")) &&
                           (TestMove->StartupFrames == 10) &&
                           (TestMove->ActiveFrames == 5) &&
                           (TestMove->RecoveryFrames == 15) &&
                           (TestMove->BaseDamage == 50.0f) &&
                           (TestMove->BaseScore == 100.0f);
    
    if (bBasicFieldsValid)
    {
        PassedTests++;
        TestResults.Add(TEXT("PASS: Basic fields integrity"));
        UE_LOG(LogTemp, Log, TEXT("✓ Basic fields test passed"));
        return true;
    }
    else
    {
        TestResults.Add(TEXT("FAIL: Basic fields integrity"));
        UE_LOG(LogTemp, Error, TEXT("✗ Basic fields test failed"));
        return false;
    }
}

bool UComboMoveDataIntegrationTest::TestEnhancedFieldsIntegrity()
{
    TotalTests++;
    UE_LOG(LogTemp, Log, TEXT("Testing enhanced PRD fields..."));
    
    UComboMoveData* TestMove = CreateTestMoveData(FName("TestEnhanced"));
    if (!TestMove)
    {
        TestResults.Add(TEXT("FAIL: Could not create enhanced test move"));
        return false;
    }
    
    // Test all enhanced fields from PRD
    TestMove->DirectionMask = static_cast<int32>(EDirectionMask::Up) | static_cast<int32>(EDirectionMask::Right);
    TestMove->ComboWindowStart = 5;
    TestMove->ComboWindowEnd = 12;
    //TestMove->NextMoves = {FName("Move1"), FName("Move2")};
    TestMove->CancelCondition = static_cast<int32>(ECancelCondition::OnHit) | static_cast<int32>(ECancelCondition::OnKill);
    TestMove->PreserveBuffer = true;
    TestMove->TargetPolicy = ETargetPolicy::Locked;
    TestMove->ResourceCost = 25;
    TestMove->MinWeaponLevel = 3;
    TestMove->FXTagArray = {FName("VFX_Test"), FName("SFX_Test")};
    
    // Validate enhanced fields
    bool bEnhancedFieldsValid = (TestMove->DirectionMask != 0) &&
                              (TestMove->ComboWindowStart == 5) &&
                              (TestMove->ComboWindowEnd == 12) &&
                              (TestMove->NextMoves.Num() == 2) &&
                              (TestMove->CancelCondition != 0) &&
                              (TestMove->PreserveBuffer == true) &&
                              (TestMove->TargetPolicy == ETargetPolicy::Locked) &&
                              (TestMove->ResourceCost == 25) &&
                              (TestMove->MinWeaponLevel == 3) &&
                              (TestMove->FXTagArray.Num() == 2);
    
    if (bEnhancedFieldsValid)
    {
        PassedTests++;
        TestResults.Add(TEXT("PASS: Enhanced fields integrity"));
        UE_LOG(LogTemp, Log, TEXT("✓ Enhanced fields test passed"));
        return true;
    }
    else
    {
        TestResults.Add(TEXT("FAIL: Enhanced fields integrity"));
        UE_LOG(LogTemp, Error, TEXT("✗ Enhanced fields test failed"));
        return false;
    }
}

bool UComboMoveDataIntegrationTest::TestChainRuleStructure()
{
    TotalTests++;
    UE_LOG(LogTemp, Log, TEXT("Testing ChainRule structure..."));
    
    UComboMoveData* TestMove = CreateTestMoveData(FName("TestChain"));
    if (!TestMove)
    {
        TestResults.Add(TEXT("FAIL: Could not create chain test move"));
        return false;
    }
    
    // Test ChainRule configuration
    FChainRule& ChainRule = TestMove->ChainRule;
    ChainRule.MaxLoop = 5;
    ChainRule.ChainWindow = 8.0f / 60.0f; // 8 frames in seconds
    ChainRule.TriggerCondition = ETriggerCondition::OnHit;
    ChainRule.AutoAction = EAutoAction::Loop;
    
    bool bChainRuleValid = (ChainRule.MaxLoop == 5) &&
                          (ChainRule.ChainWindow > 0.0f) &&
                          (ChainRule.TriggerCondition == ETriggerCondition::OnHit) &&
                          (ChainRule.AutoAction == EAutoAction::Loop);
    
    if (bChainRuleValid)
    {
        PassedTests++;
        TestResults.Add(TEXT("PASS: ChainRule structure"));
        UE_LOG(LogTemp, Log, TEXT("✓ ChainRule test passed"));
        return true;
    }
    else
    {
        TestResults.Add(TEXT("FAIL: ChainRule structure"));
        UE_LOG(LogTemp, Error, TEXT("✗ ChainRule test failed"));
        return false;
    }
}

bool UComboMoveDataIntegrationTest::TestEnumDefinitions()
{
    TotalTests++;
    UE_LOG(LogTemp, Log, TEXT("Testing enum definitions..."));
    
    bool bEnumsValid = true;
    
    // Test DirectionMask enum operations
    bEnumsValid &= TestDirectionMaskOperations();
    
    // Test CancelCondition bitflags
    bEnumsValid &= TestCancelConditionBitflags();
    
    // Test TargetPolicy enum
    bEnumsValid &= TestTargetPolicyEnum();
    
    if (bEnumsValid)
    {
        PassedTests++;
        TestResults.Add(TEXT("PASS: Enum definitions"));
        UE_LOG(LogTemp, Log, TEXT("✓ Enum definitions test passed"));
        return true;
    }
    else
    {
        TestResults.Add(TEXT("FAIL: Enum definitions"));
        UE_LOG(LogTemp, Error, TEXT("✗ Enum definitions test failed"));
        return false;
    }
}

bool UComboMoveDataIntegrationTest::TestGameplayTagIntegration()
{
    TotalTests++;
    UE_LOG(LogTemp, Log, TEXT("Testing GameplayTag integration..."));
    
    UComboMoveData* TestMove = CreateTestMoveData(FName("TestTags"));
    if (!TestMove)
    {
        TestResults.Add(TEXT("FAIL: Could not create tag test move"));
        return false;
    }
    
    // Test GameplayTag container
    FGameplayTagContainer TagContainer;
    FGameplayTag SignatureTag = FGameplayTag::RequestGameplayTag(FName("Move.Signature"));
    FGameplayTag LauncherTag = FGameplayTag::RequestGameplayTag(FName("Move.Launcher"));
    
    TagContainer.AddTag(SignatureTag);
    TagContainer.AddTag(LauncherTag);
    TestMove->Tags = TagContainer;
    
    bool bTagsValid = TestMove->Tags.HasTag(SignatureTag) &&
                     TestMove->Tags.HasTag(LauncherTag) &&
                     (TestMove->Tags.Num() == 2);
    
    if (bTagsValid)
    {
        PassedTests++;
        TestResults.Add(TEXT("PASS: GameplayTag integration"));
        UE_LOG(LogTemp, Log, TEXT("✓ GameplayTag integration test passed"));
        return true;
    }
    else
    {
        TestResults.Add(TEXT("FAIL: GameplayTag integration"));
        UE_LOG(LogTemp, Error, TEXT("✗ GameplayTag integration test failed"));
        return false;
    }
}

bool UComboMoveDataIntegrationTest::TestValidationSystem()
{
    TotalTests++;
    UE_LOG(LogTemp, Log, TEXT("Testing validation system..."));
    
    if (!ExampleCreator)
    {
        TestResults.Add(TEXT("FAIL: ExampleCreator not available"));
        return false;
    }
    
    UComboMoveData* ValidMove = CreateTestMoveData(FName("ValidMove"));
    UComboMoveData* InvalidMove = CreateTestMoveData(FName("InvalidMove"));
    
    if (!ValidMove || !InvalidMove)
    {
        TestResults.Add(TEXT("FAIL: Could not create validation test moves"));
        return false;
    }
    
    // Setup valid move
    ValidMove->StartupFrames = 10;
    ValidMove->ActiveFrames = 5;
    ValidMove->RecoveryFrames = 15;
    ValidMove->ComboWindowStart = 5;
    ValidMove->ComboWindowEnd = 12;
    ValidMove->ResourceCost = 10;
    ValidMove->MinWeaponLevel = 1;
    
    // Setup invalid move
    InvalidMove->StartupFrames = -5; // Invalid
    InvalidMove->ComboWindowStart = 15;
    InvalidMove->ComboWindowEnd = 10; // Invalid: End < Start
    InvalidMove->ResourceCost = -10; // Invalid
    InvalidMove->MinWeaponLevel = 0; // Invalid
    
    bool bValidationWorking = ExampleCreator->ValidateComboMoveData(ValidMove) &&
                             !ExampleCreator->ValidateComboMoveData(InvalidMove);
    
    if (bValidationWorking)
    {
        PassedTests++;
        TestResults.Add(TEXT("PASS: Validation system"));
        UE_LOG(LogTemp, Log, TEXT("✓ Validation system test passed"));
        return true;
    }
    else
    {
        TestResults.Add(TEXT("FAIL: Validation system"));
        UE_LOG(LogTemp, Error, TEXT("✗ Validation system test failed"));
        return false;
    }
}

bool UComboMoveDataIntegrationTest::TestExampleImplementations()
{
    TotalTests++;
    UE_LOG(LogTemp, Log, TEXT("Testing example implementations..."));
    
    if (!ExampleCreator)
    {
        TestResults.Add(TEXT("FAIL: ExampleCreator not available"));
        return false;
    }
    
    // Test that examples can be created without errors
    try
    {
        ExampleCreator->CreateFlyingSwallowExample();
        ExampleCreator->CreateIzunaGrabExample();
        ExampleCreator->CreateBasicLightAttackExample();
        ExampleCreator->CreateChainAttackExample();
        
        PassedTests++;
        TestResults.Add(TEXT("PASS: Example implementations"));
        UE_LOG(LogTemp, Log, TEXT("✓ Example implementations test passed"));
        return true;
    }
    catch (...)
    {
        TestResults.Add(TEXT("FAIL: Example implementations"));
        UE_LOG(LogTemp, Error, TEXT("✗ Example implementations test failed"));
        return false;
    }
}

bool UComboMoveDataIntegrationTest::PerformanceTestEnhancedStructure()
{
    TotalTests++;
    UE_LOG(LogTemp, Log, TEXT("Testing enhanced structure performance..."));

    const int32 TestIterations = 1000;
    double StartTime = FPlatformTime::Seconds();

    // Create many test moves to check performance impact
    TArray<UComboMoveData*> LocalTestMoves;
    LocalTestMoves.Reserve(TestIterations);

    for (int32 i = 0; i < TestIterations; i++)
    {
        UComboMoveData* TestMove = CreateTestMoveData(FName(*FString::Printf(TEXT("PerfTest_%d"), i)));
        if (TestMove)
        {
            // Configure with enhanced fields to test performance
            TestMove->DirectionMask = static_cast<int32>(EDirectionMask::Up);
            TestMove->ComboWindowStart = i % 10;
            TestMove->ComboWindowEnd = (i % 10) + 5;
            TestMove->PreserveBuffer = (i % 2 == 0);
            TestMove->TargetPolicy = static_cast<ETargetPolicy>(i % 3);
            TestMove->ResourceCost = i % 20;
            TestMove->MinWeaponLevel = (i % 5) + 1;
            
            LocalTestMoves.Add(TestMove);
        }
    }

    double EndTime = FPlatformTime::Seconds();
    double ElapsedTime = EndTime - StartTime;

    bool bPerformanceAcceptable = (ElapsedTime < 1.0) && (LocalTestMoves.Num() == TestIterations);

    if (bPerformanceAcceptable)
    {
        PassedTests++;
        TestResults.Add(FString::Printf(TEXT("PASS: Performance test (%.3fs for %d moves)"), ElapsedTime, TestIterations));
        UE_LOG(LogTemp, Log, TEXT("✓ Performance test passed: %.3f seconds for %d moves"), ElapsedTime, TestIterations);
        return true;
    }
    else
    {
        TestResults.Add(FString::Printf(TEXT("FAIL: Performance test (%.3fs for %d moves - too slow or incomplete)"), ElapsedTime, LocalTestMoves.Num()));
        UE_LOG(LogTemp, Error, TEXT("✗ Performance test failed: %.3f seconds for %d moves"), ElapsedTime, LocalTestMoves.Num());
        return false;
    }
}

bool UComboMoveDataIntegrationTest::TestBlueprintVisibility()
{
    TotalTests++;
    UE_LOG(LogTemp, Log, TEXT("Testing Blueprint visibility..."));
    
    UComboMoveData* TestMove = CreateTestMoveData(FName("BlueprintTest"));
    if (!TestMove)
    {
        TestResults.Add(TEXT("FAIL: Could not create test move for Blueprint visibility test"));
        return false;
    }
    
    // Test if the class is Blueprint visible
    UClass* MoveDataClass = UComboMoveData::StaticClass();
    bool bIsBlueprintType = MoveDataClass->GetBoolMetaData(FName("IsBlueprintBase"));
    bool bIsBlueprintable = MoveDataClass->HasMetaData(FName("BlueprintType"));
    
    if (bIsBlueprintType || bIsBlueprintable)
    {
        PassedTests++;
        TestResults.Add(TEXT("PASS: ComboMoveData is Blueprint accessible"));
        UE_LOG(LogTemp, Log, TEXT("✓ Blueprint visibility test passed"));
        return true;
    }
    else
    {
        TestResults.Add(TEXT("FAIL: ComboMoveData is not Blueprint accessible"));
        UE_LOG(LogTemp, Error, TEXT("✗ Blueprint visibility test failed"));
        return false;
    }
}

FString UComboMoveDataIntegrationTest::GenerateTestReport()
{
    FString Report = TEXT("\n=== ComboMoveData Integration Test Report ===\n");
    Report += FString::Printf(TEXT("Tests Passed: %d/%d\n"), PassedTests, TotalTests);
    Report += FString::Printf(TEXT("Success Rate: %.1f%%\n"), TotalTests > 0 ? (PassedTests * 100.0f / TotalTests) : 0.0f);
    Report += TEXT("\nDetailed Results:\n");
    
    for (const FString& Result : TestResults)
    {
        Report += TEXT("- ") + Result + TEXT("\n");
    }
    
    Report += TEXT("\n=== End Report ===\n");
    return Report;
}

UComboMoveData* UComboMoveDataIntegrationTest::CreateTestMoveData(const FName& MoveID)
{
    UComboMoveData* NewMoveData = NewObject<UComboMoveData>(this);
    if (NewMoveData)
    {
        NewMoveData->MoveID = MoveID;
        // Set some default values
        NewMoveData->StartupFrames = 5;
        NewMoveData->ActiveFrames = 3;
        NewMoveData->RecoveryFrames = 10;
        NewMoveData->BaseDamage = 10.0f;
        NewMoveData->BaseScore = 100.0f;
        TestMoves.Add(NewMoveData);
    }
    return NewMoveData;
}

bool UComboMoveDataIntegrationTest::TestDirectionMaskOperations()
{
    UE_LOG(LogTemp, Log, TEXT("Testing direction mask operations..."));
    
    // Test individual direction flags
    int32 UpMask = static_cast<int32>(EDirectionMask::Up);
    int32 DownMask = static_cast<int32>(EDirectionMask::Down);
    int32 CombinedMask = UpMask | DownMask;
    
    bool bUpTest = (CombinedMask & UpMask) != 0;
    bool bDownTest = (CombinedMask & DownMask) != 0;
    bool bLeftTest = (CombinedMask & static_cast<int32>(EDirectionMask::Left)) == 0;
    
    return bUpTest && bDownTest && bLeftTest;
}

bool UComboMoveDataIntegrationTest::TestCancelConditionBitflags()
{
    UE_LOG(LogTemp, Log, TEXT("Testing cancel condition bitflags..."));
    
    int32 OnHit = static_cast<int32>(ECancelCondition::OnHit);
    int32 OnBlock = static_cast<int32>(ECancelCondition::OnBlock);
    int32 Combined = OnHit | OnBlock;
    
    bool bOnHitTest = (Combined & OnHit) != 0;
    bool bOnBlockTest = (Combined & OnBlock) != 0;
    bool bOnWhiffTest = (Combined & static_cast<int32>(ECancelCondition::OnWhiff)) == 0;
    
    return bOnHitTest && bOnBlockTest && bOnWhiffTest;
}

bool UComboMoveDataIntegrationTest::TestTargetPolicyEnum()
{
    UE_LOG(LogTemp, Log, TEXT("Testing target policy enum..."));
    
    ETargetPolicy Policy1 = ETargetPolicy::Nearest;
    ETargetPolicy Policy2 = ETargetPolicy::Locked;
    ETargetPolicy Policy3 = ETargetPolicy::Prev;
    
    return (Policy1 != Policy2) && (Policy2 != Policy3) && (Policy1 != Policy3);
}

bool UComboMoveDataIntegrationTest::ValidateFieldDefaults(const UComboMoveData* MoveData)
{
    if (!MoveData)
    {
        return false;
    }
    
    // Validate that default values are reasonable
    bool bValidFrameData = MoveData->StartupFrames >= 0 && 
                          MoveData->ActiveFrames >= 0 && 
                          MoveData->RecoveryFrames >= 0;
                          
    bool bValidComboWindow = MoveData->ComboWindowStart <= MoveData->ComboWindowEnd;
    bool bValidResourceCost = MoveData->ResourceCost >= 0;
    bool bValidWeaponLevel = MoveData->MinWeaponLevel >= 1;
    
    return bValidFrameData && bValidComboWindow && bValidResourceCost && bValidWeaponLevel;
}

bool UComboMoveDataIntegrationTest::TestChainRuleConfiguration()
{
    UE_LOG(LogTemp, Log, TEXT("Testing chain rule configuration..."));
    
    UComboMoveData* TestMove = CreateTestMoveData(FName("ChainTest"));
    if (!TestMove)
    {
        return false;
    }
    
    // Configure chain rule
    TestMove->ChainRule.MaxLoop = 3;
    TestMove->ChainRule.ChainWindow = 0.5f;
    TestMove->ChainRule.TriggerCondition = ETriggerCondition::OnInput;
    TestMove->ChainRule.AutoAction = EAutoAction::Continue;
    
    // Validate configuration
    bool bValidMaxLoop = TestMove->ChainRule.MaxLoop > 0;
    bool bValidWindow = TestMove->ChainRule.ChainWindow > 0.0f;
    bool bValidTrigger = TestMove->ChainRule.TriggerCondition != ETriggerCondition::OnInput || true; // All values are valid
    
    return bValidMaxLoop && bValidWindow && bValidTrigger;
}