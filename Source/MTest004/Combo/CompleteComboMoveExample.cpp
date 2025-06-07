// CompleteComboMoveExample.cpp
#include "CompleteComboMoveExample.h"
#include "Engine/Engine.h"
#include "GameplayTagsManager.h"

UCompleteComboMoveExample::UCompleteComboMoveExample()
{
}

void UCompleteComboMoveExample::CreateFlyingSwallowExample()
{
    // Create example Flying Swallow move data showcasing all PRD fields
    UComboMoveData* FlyingSwallow = NewObject<UComboMoveData>();
    
    // Basic Properties
    FlyingSwallow->MoveID = FName("Flying_Swallow");
    FlyingSwallow->InputPattern = {ECombatInputKey::CIK_Heavy, ECombatInputKey::CIK_Jump}; // 示例：重击+跳跃
    FlyingSwallow->InputFlexBits = 0; // No flexibility for signature move
    
    // Direction restrictions - only forward allowed
    SetupDirectionMask(FlyingSwallow, {EDirectionMask::Up}); // Forward/Up
    
    // Valid states
    FlyingSwallow->ValidStates.Add(ECombatState::CS_Air);
    FlyingSwallow->ValidStates.Add(ECombatState::CS_Ground);
    
    // Frame data (按PRD Flying Swallow规格)
    SetupBasicMoveProperties(FlyingSwallow, FName("Flying_Swallow"), 12, 8, 22);
    
    // Combo window within recovery frames
    FlyingSwallow->ComboWindowStart = 4;  // Recovery子区段开始
    FlyingSwallow->ComboWindowEnd = 18;   // Recovery子区段结束
      // Next moves in chain - NOTE: These should be set after all moves are created
    // to properly reference actual UComboMoveData objects instead of FNames
    // FlyingSwallow->NextMoves = {TWeakObjectPtr<const UComboMoveData>(FlyingSwallowLand), TWeakObjectPtr<const UComboMoveData>(AirDash)};
    // For now, NextMoves will be empty until proper object references are available
    
    // Cancel conditions - can cancel on hit for chain extension
    SetupCancelConditions(FlyingSwallow, {ECancelCondition::OnHit, ECancelCondition::OnKill});
    
    // Chain rule setup for Flying Swallow
    SetupChainRule(FlyingSwallow->ChainRule, 3, ETriggerCondition::OnHit, 12.0f / 60.0f);
    FlyingSwallow->ChainRule.AutoAction = EAutoAction::Continue; // 自动继续链式
    
    // Advanced properties
    FlyingSwallow->PreserveBuffer = true; // 保留缓冲区状态切换
    FlyingSwallow->TargetPolicy = ETargetPolicy::Nearest; // 选择最近目标
    FlyingSwallow->ResourceCost = 15; // 查克拉消耗
    
    // Damage and scaling
    FlyingSwallow->BaseDamage = 85.0f;
    // DamageScaling 需要在编辑器中设置曲线资产
    
    // Gameplay tags
    FGameplayTagContainer TagContainer;
    TagContainer.AddTag(FGameplayTag::RequestGameplayTag(FName("Move.Signature")));
    TagContainer.AddTag(FGameplayTag::RequestGameplayTag(FName("Move.GapCloser")));
    TagContainer.AddTag(FGameplayTag::RequestGameplayTag(FName("Move.Aerial")));
    FlyingSwallow->Tags = TagContainer;
    
    // Requirements
    FlyingSwallow->MinWeaponLevel = 1; // 基础武器即可
    
    // Effects
    FlyingSwallow->FXTagArray = {FName("VFX_Flying_Swallow"), FName("SFX_Whoosh_Heavy")};
    
    // Scoring
    FlyingSwallow->BaseScore = 120.0f; // 高分招式
    
    UE_LOG(LogTemp, Warning, TEXT("Created Flying Swallow example with complete PRD fields"));
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, 
            TEXT("Flying Swallow Example Created - Check log for details"));
    }
}

void UCompleteComboMoveExample::CreateIzunaGrabExample()
{
    UComboMoveData* IzunaGrab = NewObject<UComboMoveData>();
    
    // Basic Properties
    IzunaGrab->MoveID = FName("Izuna_Grab");
    IzunaGrab->InputPattern = {ECombatInputKey::CIK_Heavy}; // Hold Heavy (simplified)
    IzunaGrab->InputFlexBits = 0;
    
    // Only usable in air
    IzunaGrab->ValidStates.Add(ECombatState::CS_Air);
    
    // Frame data according to PRD
    SetupBasicMoveProperties(IzunaGrab, FName("Izuna_Grab"), 4, 2, 28);
    
    // No combo window - this is a grab
    IzunaGrab->ComboWindowStart = 0;
    IzunaGrab->ComboWindowEnd = 0;
      // Next move is automatic (Izuna Drop) - NOTE: Should reference actual object
    // IzunaGrab->NextMoves = {TWeakObjectPtr<const UComboMoveData>(IzunaDrop)};
    // For now, NextMoves will be empty until proper object references are available
    
    // Can only cancel on hit (successful grab)
    SetupCancelConditions(IzunaGrab, {ECancelCondition::OnHit});
    
    // No chain rule for grabs
    IzunaGrab->ChainRule.MaxLoop = 1;
    IzunaGrab->ChainRule.TriggerCondition = ETriggerCondition::OnHit;
    
    // Target properties
    IzunaGrab->TargetPolicy = ETargetPolicy::Locked; // 锁定目标
    IzunaGrab->ResourceCost = 25; // 高消耗
    
    // Damage
    IzunaGrab->BaseDamage = 120.0f; // High damage grab
    
    // Tags
    FGameplayTagContainer TagContainer;
    TagContainer.AddTag(FGameplayTag::RequestGameplayTag(FName("Move.Signature")));
    TagContainer.AddTag(FGameplayTag::RequestGameplayTag(FName("Move.Grab")));
    TagContainer.AddTag(FGameplayTag::RequestGameplayTag(FName("Move.Aerial")));
    IzunaGrab->Tags = TagContainer;
    
    // Requirements
    IzunaGrab->MinWeaponLevel = 2; // 需要更高武器等级
    
    // Effects
    IzunaGrab->FXTagArray = {FName("VFX_Grab_Initiate"), FName("SFX_Grab_Success")};
    
    // High score for difficult move
    IzunaGrab->BaseScore = 150.0f;
    
    UE_LOG(LogTemp, Warning, TEXT("Created Izuna Grab example"));
}

void UCompleteComboMoveExample::CreateBasicLightAttackExample()
{
    UComboMoveData* LightAttack = NewObject<UComboMoveData>();
    
    // Basic Properties
    LightAttack->MoveID = FName("Light_A1");
    LightAttack->InputPattern = {ECombatInputKey::CIK_Light};
    LightAttack->InputFlexBits = 1; // 允许一定灵活性
    
    // Direction - any direction allowed
    LightAttack->DirectionMask = static_cast<int32>(EDirectionMask::Up | EDirectionMask::Down | 
                                                    EDirectionMask::Left | EDirectionMask::Right |
                                                    EDirectionMask::Neutral);
    
    // Ground state only
    LightAttack->ValidStates.Add(ECombatState::CS_Ground);
    
    // Fast startup, medium active, quick recovery
    SetupBasicMoveProperties(LightAttack, FName("Light_A1"), 6, 4, 8);
    
    // Generous combo window
    LightAttack->ComboWindowStart = 2;
    LightAttack->ComboWindowEnd = 6;
      // Can chain to other light attacks or heavy finisher - NOTE: Should reference actual objects
    // LightAttack->NextMoves = {TWeakObjectPtr<const UComboMoveData>(LightA2), TWeakObjectPtr<const UComboMoveData>(HeavyFinisher)};
    // For now, NextMoves will be empty until proper object references are available
    
    // Can cancel on various conditions
    SetupCancelConditions(LightAttack, {ECancelCondition::OnHit, ECancelCondition::OnBlock, ECancelCondition::OnWhiff});
    
    // No special chain rule
    LightAttack->ChainRule.MaxLoop = 1;
    LightAttack->ChainRule.TriggerCondition = ETriggerCondition::OnInput;
    
    // Basic properties
    LightAttack->PreserveBuffer = false;
    LightAttack->TargetPolicy = ETargetPolicy::Nearest;
    LightAttack->ResourceCost = 0; // No cost for basic attacks
    
    // Low damage, designed for combos
    LightAttack->BaseDamage = 25.0f;
    
    // Basic tags
    FGameplayTagContainer TagContainer;
    TagContainer.AddTag(FGameplayTag::RequestGameplayTag(FName("Move.Basic")));
    TagContainer.AddTag(FGameplayTag::RequestGameplayTag(FName("Move.Light")));
    LightAttack->Tags = TagContainer;
    
    // No level requirement
    LightAttack->MinWeaponLevel = 1;
    
    // Simple effects
    LightAttack->FXTagArray = {FName("VFX_Hit_Light"), FName("SFX_Swing_Light")};
    
    // Standard score
    LightAttack->BaseScore = 50.0f;
    
    UE_LOG(LogTemp, Warning, TEXT("Created Basic Light Attack example"));
}

void UCompleteComboMoveExample::CreateChainAttackExample()
{
    UComboMoveData* ChainAttack = NewObject<UComboMoveData>();
    
    // Basic Properties
    ChainAttack->MoveID = FName("Chain_Slash");
    ChainAttack->InputPattern = {ECombatInputKey::CIK_Light, ECombatInputKey::CIK_Light, ECombatInputKey::CIK_Light};
    ChainAttack->InputFlexBits = 2; // 允许第2个输入有灵活性
    
    // Any state except defensive
    ChainAttack->ValidStates.Add(ECombatState::CS_Ground);
    ChainAttack->ValidStates.Add(ECombatState::CS_Air);
    ChainAttack->ValidStates.Add(ECombatState::CS_Run);
    
    SetupBasicMoveProperties(ChainAttack, FName("Chain_Slash"), 8, 12, 16);
    
    // Extended combo window for chain continuation
    ChainAttack->ComboWindowStart = 6;
    ChainAttack->ComboWindowEnd = 14;
    
    // Can continue into itself for chain
    // Chain connections - NOTE: Should reference actual objects
    // ChainAttack->NextMoves = {TWeakObjectPtr<const UComboMoveData>(ChainSlash), TWeakObjectPtr<const UComboMoveData>(ChainFinisher)};
    // For now, NextMoves will be empty until proper object references are available
    
    // Cancel on hit to continue chain
    SetupCancelConditions(ChainAttack, {ECancelCondition::OnHit});
    
    // Advanced chain rule
    SetupChainRule(ChainAttack->ChainRule, 5, ETriggerCondition::OnHit, 8.0f / 60.0f);
    ChainAttack->ChainRule.AutoAction = EAutoAction::Loop;
    
    // Chain-specific properties
    ChainAttack->PreserveBuffer = true; // 保持链式状态
    ChainAttack->TargetPolicy = ETargetPolicy::Prev; // 继续攻击同一目标
    ChainAttack->ResourceCost = 5; // 每次链式消耗
    
    ChainAttack->BaseDamage = 40.0f;
    
    // Chain tags
    FGameplayTagContainer TagContainer;
    TagContainer.AddTag(FGameplayTag::RequestGameplayTag(FName("Move.Chain")));
    TagContainer.AddTag(FGameplayTag::RequestGameplayTag(FName("Move.Multi")));
    ChainAttack->Tags = TagContainer;
    
    ChainAttack->MinWeaponLevel = 2;
    
    // Chain effects
    ChainAttack->FXTagArray = {FName("VFX_Chain_Slash"), FName("SFX_Chain_Hit"), FName("VFX_Chain_Trail")};
    
    ChainAttack->BaseScore = 80.0f;
    
    UE_LOG(LogTemp, Warning, TEXT("Created Chain Attack example"));
}

bool UCompleteComboMoveExample::ValidateComboMoveData(const UComboMoveData* MoveData) const
{
    if (!MoveData)
    {
        UE_LOG(LogTemp, Error, TEXT("MoveData is null"));
        return false;
    }
    
    // Validate basic requirements
    if (MoveData->MoveID.IsNone())
    {
        UE_LOG(LogTemp, Error, TEXT("MoveID is empty"));
        return false;
    }
    
    if (MoveData->InputPattern.Num() == 0)
    {
        UE_LOG(LogTemp, Error, TEXT("InputPattern is empty"));
        return false;
    }
    
    if (MoveData->ValidStates.Num() == 0)
    {
        UE_LOG(LogTemp, Error, TEXT("ValidStates is empty"));
        return false;
    }
    
    // Validate frame data
    if (MoveData->StartupFrames < 0 || MoveData->ActiveFrames < 0 || MoveData->RecoveryFrames < 0)
    {
        UE_LOG(LogTemp, Error, TEXT("Invalid frame data"));
        return false;
    }
    
    // Validate combo window
    if (MoveData->ComboWindowStart > MoveData->ComboWindowEnd)
    {
        UE_LOG(LogTemp, Error, TEXT("ComboWindowStart > ComboWindowEnd"));
        return false;
    }
    
    if (MoveData->ComboWindowEnd > MoveData->RecoveryFrames)
    {
        UE_LOG(LogTemp, Warning, TEXT("ComboWindowEnd extends beyond RecoveryFrames"));
    }
    
    // Validate resource cost
    if (MoveData->ResourceCost < 0)
    {
        UE_LOG(LogTemp, Error, TEXT("Invalid ResourceCost"));
        return false;
    }
    
    // Validate weapon level
    if (MoveData->MinWeaponLevel < 1)
    {
        UE_LOG(LogTemp, Error, TEXT("MinWeaponLevel must be at least 1"));
        return false;
    }
    
    UE_LOG(LogTemp, Log, TEXT("MoveData validation passed for %s"), *MoveData->MoveID.ToString());
    return true;
}

FString UCompleteComboMoveExample::GetMoveDataDebugInfo(const UComboMoveData* MoveData) const
{
    if (!MoveData)
    {
        return TEXT("MoveData is null");
    }
    
    FString DebugInfo = FString::Printf(TEXT("=== %s Debug Info ===\n"), *MoveData->MoveID.ToString());
    
    // Basic info
    DebugInfo += FString::Printf(TEXT("Input Pattern: %d keys\n"), MoveData->InputPattern.Num());
    DebugInfo += FString::Printf(TEXT("Input Flex Bits: %d\n"), MoveData->InputFlexBits);
    DebugInfo += FString::Printf(TEXT("Direction Mask: %d\n"), MoveData->DirectionMask);
    
    // Frame data
    DebugInfo += FString::Printf(TEXT("Frames: %d/%d/%d (S/A/R)\n"), 
                                MoveData->StartupFrames, MoveData->ActiveFrames, MoveData->RecoveryFrames);
    DebugInfo += FString::Printf(TEXT("Combo Window: %d-%d\n"), 
                                MoveData->ComboWindowStart, MoveData->ComboWindowEnd);    // Chain info
    int32 ValidNextMoves = 0;
    for (const TWeakObjectPtr<const UComboMoveData>& NextMovePtr : MoveData->NextMoves)
    {
        if (NextMovePtr.IsValid())
        {
            ValidNextMoves++;
        }
    }
    DebugInfo += FString::Printf(TEXT("Next Moves: %d (%d valid)\n"), MoveData->NextMoves.Num(), ValidNextMoves);
    DebugInfo += FString::Printf(TEXT("Next Moves IDs: %d\n"), MoveData->NextMovesID.Num());
    DebugInfo += FString::Printf(TEXT("Chain Max Loops: %d\n"), MoveData->ChainRule.MaxLoop);
    DebugInfo += FString::Printf(TEXT("Cancel Condition: %d\n"), MoveData->CancelCondition);
    
    // Properties
    DebugInfo += FString::Printf(TEXT("Resource Cost: %d\n"), MoveData->ResourceCost);
    DebugInfo += FString::Printf(TEXT("Base Damage: %.1f\n"), MoveData->BaseDamage);
    DebugInfo += FString::Printf(TEXT("Min Weapon Level: %d\n"), MoveData->MinWeaponLevel);
    DebugInfo += FString::Printf(TEXT("Base Score: %.1f\n"), MoveData->BaseScore);
    
    // Advanced properties
    DebugInfo += FString::Printf(TEXT("Preserve Buffer: %s\n"), MoveData->PreserveBuffer ? TEXT("Yes") : TEXT("No"));
    DebugInfo += FString::Printf(TEXT("Target Policy: %s\n"), 
                                MoveData->TargetPolicy == ETargetPolicy::Nearest ? TEXT("Nearest") :
                                MoveData->TargetPolicy == ETargetPolicy::Locked ? TEXT("Locked") : TEXT("Prev"));
    
    // Tags and effects
    DebugInfo += FString::Printf(TEXT("Gameplay Tags: %d\n"), MoveData->Tags.Num());
    DebugInfo += FString::Printf(TEXT("FX Tags: %d\n"), MoveData->FXTagArray.Num());
    
    return DebugInfo;
}

// Helper function implementations
void UCompleteComboMoveExample::SetupBasicMoveProperties(UComboMoveData* MoveData, const FName& ID, 
                                                         int32 StartupF, int32 ActiveF, int32 RecoveryF)
{
    if (!MoveData) return;
    
    MoveData->MoveID = ID;
    MoveData->StartupFrames = StartupF;
    MoveData->ActiveFrames = ActiveF;
    MoveData->RecoveryFrames = RecoveryF;
}

void UCompleteComboMoveExample::SetupChainRule(FChainRule& ChainRule, int32 MaxLoops, 
                                               ETriggerCondition Trigger, float WindowSeconds)
{
    ChainRule.MaxLoop = MaxLoops;
    ChainRule.TriggerCondition = Trigger;
    ChainRule.ChainWindow = WindowSeconds;
}

void UCompleteComboMoveExample::SetupCancelConditions(UComboMoveData* MoveData, 
                                                      TArray<ECancelCondition> Conditions)
{
    if (!MoveData) return;
    
    int32 CombinedConditions = 0;
    for (ECancelCondition Condition : Conditions)
    {
        CombinedConditions |= static_cast<int32>(Condition);
    }
    MoveData->CancelCondition = CombinedConditions;
}

void UCompleteComboMoveExample::SetupDirectionMask(UComboMoveData* MoveData, 
                                                   TArray<EDirectionMask> AllowedDirections)
{
    if (!MoveData) return;
    
    int32 CombinedMask = 0;
    for (EDirectionMask Direction : AllowedDirections)
    {
        CombinedMask |= static_cast<int32>(Direction);
    }
    MoveData->DirectionMask = CombinedMask;
}
