#pragma once
#include "Engine/DataAsset.h"
#include "CombatInputBuffer.h"
#include "CombatStateMachine.h" // Assuming ECombatState is used, otherwise this might also need re-evaluation
#include "GameplayTagContainer.h" // For FGameplayTagContainer
#include "Engine/CurveTable.h" // For UCurveFloat
#include "ComboMoveData.generated.h"

// Forward declaration
class UComboScorer;

// PRD: Direction Mask for input restrictions
UENUM(BlueprintType, meta = (Bitflags, UseEnumValuesAsMaskValuesInEditor = "true"))
enum class EDirectionMask : uint8
{
    None     = 0,
    Up       = 1 << 0,  // ↑
    Down     = 1 << 1,  // ↓
    Left     = 1 << 2,  // ←
    Right    = 1 << 3,  // →
    Neutral  = 1 << 4   // 中立
};
ENUM_CLASS_FLAGS(EDirectionMask)

// PRD: Cancel Condition bitflags
UENUM(BlueprintType, meta = (Bitflags, UseEnumValuesAsMaskValuesInEditor = "true"))
enum class ECancelCondition : uint8
{
    None     = 0,
    OnHit    = 1 << 0,
    OnBlock  = 1 << 1,
    OnWhiff  = 1 << 2,
    OnKill   = 1 << 3
};
ENUM_CLASS_FLAGS(ECancelCondition)

// PRD: Target Policy
UENUM(BlueprintType)
enum class ETargetPolicy : uint8
{
    Nearest,
    Locked,
    Prev
};

// PRD: Trigger Condition for Chain Rules
UENUM(BlueprintType)
enum class ETriggerCondition : uint8
{
    OnInput,     // 输入触发
    OnHit,       // 命中触发
    OnBlock,     // 格挡触发
    OnKill,      // 击杀触发
    OnTimer      // 定时器触发
};

// PRD: Auto Action for Chain Rules
UENUM(BlueprintType)
enum class EAutoAction : uint8
{
    None,
    Continue,    // 自动继续
    Loop,        // 自动循环
    Break        // 自动终止
};

USTRUCT(BlueprintType)
struct FChainRule
{
    GENERATED_BODY()

    UPROPERTY(EditDefaultsOnly, Category = "Chain", meta = (DisplayName = "Max Loop Count", ToolTip = "Maximum chain loops (飞燕 Lv1=1, Lv2=2, Lv3=3)"))
    int32 MaxLoop = 1;

    UPROPERTY(EditDefaultsOnly, Category = "Chain", meta = (DisplayName = "Chain Window (Frames)", ToolTip = "Input valid window in logic frames"))
    float ChainWindow = 0.2f; // 单位秒

    UPROPERTY(EditDefaultsOnly, Category = "Chain", meta = (DisplayName = "Trigger Condition", ToolTip = "When this chain rule activates"))
    ETriggerCondition TriggerCondition = ETriggerCondition::OnInput;

    UPROPERTY(EditDefaultsOnly, Category = "Chain", meta = (DisplayName = "Auto Action", ToolTip = "Optional auto action when triggered"))
    EAutoAction AutoAction = EAutoAction::None;

    UPROPERTY(EditDefaultsOnly, Category = "Chain")
    TEnumAsByte<ECollisionChannel> TargetChannel;
};

UCLASS(BlueprintType)
class MTEST004_API UComboMoveData : public UPrimaryDataAsset
{
    GENERATED_BODY()
public:
    UPROPERTY(EditDefaultsOnly, Category = "Combo")
    FName MoveID;

    UPROPERTY(EditDefaultsOnly, Category = "Input")
    TArray<ECombatInputKey> InputPattern;    UPROPERTY(EditDefaultsOnly, Category = "Input", meta = (DisplayName = "Flexible Input Bits", ToolTip = "Bitmask for which input positions allow flexibility"))
    int32 InputFlexBits = 0;

    UPROPERTY(EditDefaultsOnly, Category = "Input", meta = (DisplayName = "Direction Mask", ToolTip = "Allowed directional inputs", Bitmask, BitmaskEnum = "EDirectionMask"))
    int32 DirectionMask = 0;

    UPROPERTY(EditDefaultsOnly, Category = "State")
    TSet<ECombatState> ValidStates;

    UPROPERTY(EditDefaultsOnly, Category = "Timing", meta = (ClampMin = 0))
    int32 StartupFrames = 0;

    UPROPERTY(EditDefaultsOnly, Category = "Timing", meta = (ClampMin = 0))
    int32 ActiveFrames = 0;

    UPROPERTY(EditDefaultsOnly, Category = "Timing", meta = (ClampMin = 0))
    int32 RecoveryFrames = 0;

    UPROPERTY(EditDefaultsOnly, Category = "Timing", meta = (DisplayName = "Combo Window Start", ToolTip = "Frame in recovery where combo window opens", ClampMin = 0))
    int32 ComboWindowStart = 0;

    UPROPERTY(EditDefaultsOnly, Category = "Timing", meta = (DisplayName = "Combo Window End", ToolTip = "Frame in recovery where combo window closes", ClampMin = 0))
    int32 ComboWindowEnd = 0;

    UPROPERTY(EditDefaultsOnly, Category = "Chain", meta = (DisplayName = "NextLightAttackMove", ToolTip = "Normal combo chain connections"))
    TObjectPtr<UComboMoveData> NextLightAttackMove;    
      UPROPERTY(EditDefaultsOnly, Category = "Chain", meta = (DisplayName = "Next Moves", ToolTip = "Normal combo chain connections"))
    TArray<TWeakObjectPtr<const UComboMoveData>> NextMoves;
    
    UPROPERTY(EditDefaultsOnly, Category = "Chain", meta = (DisplayName = "Next Moves IDs", ToolTip = "MoveIDs of NextMoves for easy access and debugging"))
    TArray<FName> NextMovesID;
    
    UPROPERTY(EditDefaultsOnly, Category = "Cancel")
    int32 CancelMask = 0;

    UPROPERTY(EditDefaultsOnly, Category = "Cancel")
    int32 CancelReceivers = 0;

    UPROPERTY(EditDefaultsOnly, Category = "Cancel", meta = (DisplayName = "Cancel Condition", ToolTip = "Conditions that allow canceling this move", Bitmask, BitmaskEnum = "ECancelCondition"))
    int32 CancelCondition = 0;

    UPROPERTY(EditDefaultsOnly, Category = "Chain")
    FChainRule ChainRule;

    UPROPERTY(EditDefaultsOnly, Category = "Buffer", meta = (DisplayName = "Preserve Buffer", ToolTip = "Keep 1 input when switching phases"))
    bool PreserveBuffer = false;

    UPROPERTY(EditDefaultsOnly, Category = "Target", meta = (DisplayName = "Target Policy", ToolTip = "How to select targets for this move"))
    ETargetPolicy TargetPolicy = ETargetPolicy::Nearest;

    UPROPERTY(EditDefaultsOnly, Category = "Resource", meta = (DisplayName = "Resource Cost", ToolTip = "Chakra/Ki consumption", ClampMin = 0))
    int32 ResourceCost = 0;

    UPROPERTY(EditDefaultsOnly, Category = "Damage")
    float BaseDamage = 10.0f;

    UPROPERTY(EditDefaultsOnly, Category = "Damage", meta = (DisplayName = "Damage Scaling", ToolTip = "Combo depth scaling curve"))
    TObjectPtr<UCurveFloat> DamageScaling;

    UPROPERTY(EditDefaultsOnly, Category = "Tags", meta = (DisplayName = "Move Tags", ToolTip = "Gameplay tags for this move (Signature, Launcher, etc.)"))
    FGameplayTagContainer Tags;

    UPROPERTY(EditDefaultsOnly, Category = "Requirements", meta = (DisplayName = "Min Weapon Level", ToolTip = "Minimum weapon level required", ClampMin = 1))
    int32 MinWeaponLevel = 1;

    UPROPERTY(EditDefaultsOnly, Category = "Effects", meta = (DisplayName = "FX Tags", ToolTip = "VFX/SFX binding tags"))
    TArray<FName> FXTagArray;

    UPROPERTY(EditDefaultsOnly, Category = "Visual")
    TObjectPtr<UAnimMontage> AttackMontage;

    UPROPERTY(EditAnywhere)
    FName PrerequisiteTag;

    UPROPERTY(EditAnywhere)
    FName GrantTagOnHit;

    // For Simulation
    UPROPERTY(EditDefaultsOnly, Category = "Simulation|NextMove", meta = (DisplayName = "Next Light Attack (Sim)"))
    TObjectPtr<UComboMoveData> NextLightAttackMove_Sim;

    UPROPERTY(EditDefaultsOnly, Category = "Simulation|NextMove", meta = (DisplayName = "Next Heavy Attack (Sim)"))
    TObjectPtr<UComboMoveData> NextHeavyAttackMove_Sim;

    /**
     * 若非空 → 该招式需要严格按顺序执行完列表里的招式后才能触发
     * 例： {A1,A2,A3,B1,B2}
     */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Prerequisite", meta = (AllowedClasses = "UComboMoveData"))
    TArray<TSoftObjectPtr<UComboMoveData>> PrereqComboSeq;

    /** true = 执行此招式时不重置前置序列进度 */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Prerequisite")
    bool bIgnoreForSequence = false;

    UPROPERTY(EditDefaultsOnly, Category = "Scoring", meta = (DisplayName = "Custom Scorer Class"))
    TSubclassOf<UComboScorer> CustomScorerClass;    UPROPERTY(EditDefaultsOnly, Category = "Scoring", meta = (DisplayName = "Base Score for this Move"))
    float BaseScore = 100.0f; // Default base score for this move

public:
    /**
     * 同步 NextMoves 和 NextMovesID 数组
     * 当修改 NextMoves 后调用此函数来更新 NextMovesID
     */
    UFUNCTION(BlueprintCallable, Category = "Combo")
    void SyncNextMovesID();
    
    /**
     * 添加下一个招式到 NextMoves 并自动更新 NextMovesID
     */
    UFUNCTION(BlueprintCallable, Category = "Combo")
    void AddNextMove(const UComboMoveData* NextMove);
    
    /**
     * 清空 NextMoves 和 NextMovesID
     */
    UFUNCTION(BlueprintCallable, Category = "Combo")
    void ClearNextMoves();
};
