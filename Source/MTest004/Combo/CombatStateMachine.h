// CombatStateMachine.h
#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CombatStateMachine.generated.h"

UENUM(BlueprintType)
enum class ECombatState : uint8
{
    CS_Idle,
    CS_Ground,
    CS_Run,        // PRD 要求的跑步状态
    CS_Air,
    CS_Wall,
    CS_Defensive,
    CS_UTCharge,
    CS_FS_Rebound, // 飞燕反弹状态
    CS_WaterRun,   // 水上行走状态（预留）
    CS_MAX
};

USTRUCT()
struct FStateTransition
{
    GENERATED_BODY()
    
    ECombatState FromState = ECombatState::CS_Idle;
    ECombatState ToState = ECombatState::CS_Idle;
    int32 Priority = 0;
    
    // 条件检查函数指针
    TFunction<bool()> ConditionCheck;
    
    // 构造函数
    FStateTransition() = default;
    FStateTransition(ECombatState From, ECombatState To, int32 InPriority, TFunction<bool()> InCondition)
        : FromState(From), ToState(To), Priority(InPriority), ConditionCheck(InCondition) {}
};

UCLASS()
class MTEST004_API UCombatStateMachine : public UActorComponent
{
    GENERATED_BODY()
public:
    UCombatStateMachine();
    
    void UpdateState(float DeltaTime);
    void RequestStateTransition(ECombatState NewState, int32 TransitionPriority);
    void InitializeTransitionRules();
    
    ECombatState GetCurrentState() const { return CurrentState; }
    
    // 状态条件检查接口
    UFUNCTION(BlueprintCallable, Category = "State Machine")
    void SetInputPressed(bool bJump, bool bBlock, bool bWindRun);
    
    UFUNCTION(BlueprintCallable, Category = "State Machine")
    void SetMovementState(float MoveAxis, bool bIsOnFloor, bool bIsWallDetect);
    
    UFUNCTION(BlueprintCallable, Category = "State Machine")
    void SetCombatFlags(bool bParryWindowFlag, bool bIsAirborneFlag);

private:
    void ExecuteTransition(ECombatState NewState);
    
    // 状态转换条件检查函数
    bool CheckGroundToAir();
    bool CheckAirToGround();
    bool CheckGroundToRun();
    bool CheckRunToGround();
    bool CheckAnyToWall();
    bool CheckWallToAir();
    bool CheckAnyToDefensive();
    bool CheckDefensiveToGround();
    bool CheckAirToFSRebound();
    bool CheckFSReboundToAir();

    ECombatState CurrentState = ECombatState::CS_Idle;
    TArray<FStateTransition> TransitionRules;
    int32 CurrentTransitionPriority = -1;
    
    // 状态计时器和条件变量
    FTimerHandle WallTimer;
    float StateTime = 0.0f;
    float WallTimerDuration = 0.9f; // 900ms
    
    // 输入状态
    bool bJumpPressed = false;
    bool bBlockPressed = false;
    bool bWindRunPressed = false;
    
    // 移动状态
    float CurrentMoveAxis = 0.0f;
    bool bOnFloor = true;
    bool bWallDetect = false;
    
    // 战斗状态
    bool bParryWindow = false;
    bool bIsAirborne = false;
    
    // 状态持续计数器（用于帧数检查）
    int32 MoveAxisHighFrameCount = 0;
    int32 MoveAxisLowFrameCount = 0;
    
    // 飞燕反弹计时器
    FTimerHandle FSReboundTimer;
    float FSReboundDuration = 18.0f / 60.0f; // 18帧转换为秒
};
