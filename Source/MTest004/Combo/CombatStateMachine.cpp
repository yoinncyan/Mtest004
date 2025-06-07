// CombatStateMachine.cpp
#include "CombatStateMachine.h"
#include "Engine/World.h"
#include "TimerManager.h"

UCombatStateMachine::UCombatStateMachine()
{
    //PrimaryComponentTick.bCanEverTick = false; // 移除无效成员
    InitializeTransitionRules();
}

void UCombatStateMachine::InitializeTransitionRules()
{
    TransitionRules.Empty();
    
    // 根据PRD状态转换表初始化规则
    // Priority: 高数值可打断低数值
    
    // Ground → Air (优先级 3)
    TransitionRules.Add(FStateTransition(
        ECombatState::CS_Ground, 
        ECombatState::CS_Air, 
        3,
        [this]() { return CheckGroundToAir(); }
    ));
    
    // Air → Ground (自动)
    TransitionRules.Add(FStateTransition(
        ECombatState::CS_Air, 
        ECombatState::CS_Ground, 
        0,
        [this]() { return CheckAirToGround(); }
    ));
    
    // Ground ↔ Run (优先级 4)
    TransitionRules.Add(FStateTransition(
        ECombatState::CS_Ground, 
        ECombatState::CS_Run, 
        4,
        [this]() { return CheckGroundToRun(); }
    ));
    
    TransitionRules.Add(FStateTransition(
        ECombatState::CS_Run, 
        ECombatState::CS_Ground, 
        4,
        [this]() { return CheckRunToGround(); }
    ));
    
    // Any → Wall (优先级 5)
    for (int32 i = 0; i < (int32)ECombatState::CS_MAX; ++i)
    {
        ECombatState FromState = (ECombatState)i;
        if (FromState != ECombatState::CS_Wall)
        {
            TransitionRules.Add(FStateTransition(
                FromState, 
                ECombatState::CS_Wall, 
                5,
                [this]() { return CheckAnyToWall(); }
            ));
        }
    }
    
    // Wall → Air (自动)
    TransitionRules.Add(FStateTransition(
        ECombatState::CS_Wall, 
        ECombatState::CS_Air, 
        0,
        [this]() { return CheckWallToAir(); }
    ));
    
    // Any → Defensive (优先级 6)
    for (int32 i = 0; i < (int32)ECombatState::CS_MAX; ++i)
    {
        ECombatState FromState = (ECombatState)i;
        if (FromState != ECombatState::CS_Defensive)
        {
            TransitionRules.Add(FStateTransition(
                FromState, 
                ECombatState::CS_Defensive, 
                6,
                [this]() { return CheckAnyToDefensive(); }
            ));
        }
    }
    
    // Defensive → Ground (自动)
    TransitionRules.Add(FStateTransition(
        ECombatState::CS_Defensive, 
        ECombatState::CS_Ground, 
        0,
        [this]() { return CheckDefensiveToGround(); }
    ));
    
    // Air ↔ FS_Rebound (优先级 2)
    TransitionRules.Add(FStateTransition(
        ECombatState::CS_Air, 
        ECombatState::CS_FS_Rebound, 
        2,
        [this]() { return CheckAirToFSRebound(); }
    ));
    
    TransitionRules.Add(FStateTransition(
        ECombatState::CS_FS_Rebound, 
        ECombatState::CS_Air, 
        0,
        [this]() { return CheckFSReboundToAir(); }
    ));
}

void UCombatStateMachine::ExecuteTransition(ECombatState NewState)
{
    ECombatState OldState = CurrentState;
    CurrentState = NewState;
    StateTime = 0.0f;
    CurrentTransitionPriority = -1;
    
    // 状态进入逻辑
    switch (NewState)
    {
        case ECombatState::CS_Air:
            bIsAirborne = true;
            break;
        case ECombatState::CS_Ground:
        case ECombatState::CS_Run:
            bIsAirborne = false;
            break;
        case ECombatState::CS_Wall:
            // 设置墙壁计时器 900ms
            if (UWorld* World = GetWorld())
            {
                World->GetTimerManager().SetTimer(WallTimer, 
                    [this]() { 
                        // 墙壁计时器到期，自动转换到空中
                    }, 
                    WallTimerDuration, false);
            }
            break;
        case ECombatState::CS_Defensive:
            // 无敌帧 8f 的逻辑可以在这里处理
            break;
        case ECombatState::CS_FS_Rebound:
            // 设置飞燕反弹计时器 18帧
            if (UWorld* World = GetWorld())
            {
                World->GetTimerManager().SetTimer(FSReboundTimer, 
                    [this]() { 
                        // 自动回到空中状态
                        if (CurrentState == ECombatState::CS_FS_Rebound)
                        {
                            ExecuteTransition(ECombatState::CS_Air);
                        }
                    }, 
                    FSReboundDuration, false);
            }
            break;
    }
    
    UE_LOG(LogTemp, Log, TEXT("State transitioned from %d to %d"), (int32)OldState, (int32)NewState);
}

void UCombatStateMachine::UpdateState(float DeltaTime)
{
    StateTime += DeltaTime;
    
    // 更新帧计数器（假设60fps）
    const float FrameTime = 1.0f / 60.0f;
    if (DeltaTime >= FrameTime)
    {
        // 更新移动轴帧计数器
        if (CurrentMoveAxis >= 0.7f)
        {
            MoveAxisHighFrameCount++;
            MoveAxisLowFrameCount = 0;
        }
        else if (CurrentMoveAxis < 0.3f)
        {
            MoveAxisLowFrameCount++;
            MoveAxisHighFrameCount = 0;
        }
        else
        {
            // 中间值，重置计数器
            MoveAxisHighFrameCount = 0;
            MoveAxisLowFrameCount = 0;
        }
    }
    
    // 按优先级检测状态转换
    FStateTransition* BestTransition = nullptr;
    int32 HighestPriority = CurrentTransitionPriority;
    
    for (FStateTransition& Rule : TransitionRules)
    {
        if (Rule.FromState == CurrentState && Rule.ConditionCheck && Rule.ConditionCheck())
        {
            if (Rule.Priority > HighestPriority)
            {
                BestTransition = &Rule;
                HighestPriority = Rule.Priority;
            }
        }
    }
    
    if (BestTransition)
    {
        ExecuteTransition(BestTransition->ToState);
    }
}

void UCombatStateMachine::RequestStateTransition(ECombatState NewState, int32 TransitionPriority)
{
    if (TransitionPriority > CurrentTransitionPriority)
    {
        ExecuteTransition(NewState);
        CurrentTransitionPriority = TransitionPriority;
    }
}

// 状态输入接口实现
void UCombatStateMachine::SetInputPressed(bool bJump, bool bBlock, bool bWindRun)
{
    bJumpPressed = bJump;
    bBlockPressed = bBlock;
    bWindRunPressed = bWindRun;
}

void UCombatStateMachine::SetMovementState(float MoveAxis, bool bIsOnFloor, bool bIsWallDetect)
{
    CurrentMoveAxis = MoveAxis;
    this->bOnFloor = bIsOnFloor;
    this->bWallDetect = bIsWallDetect;
}

void UCombatStateMachine::SetCombatFlags(bool bParryWindowFlag, bool bIsAirborneFlag)
{
    this->bParryWindow = bParryWindowFlag;
    this->bIsAirborne = bIsAirborneFlag;
}

// 状态转换条件检查函数实现
bool UCombatStateMachine::CheckGroundToAir()
{
    // Jump 输入 或 Launcher Tag OnHit
    return bJumpPressed; // TODO: 添加 Launcher Tag 检查
}

bool UCombatStateMachine::CheckAirToGround()
{
    // bOnFloor==true
    return bOnFloor;
}

bool UCombatStateMachine::CheckGroundToRun()
{
    // MoveAxis ≥ 0.7 持续 3f
    return MoveAxisHighFrameCount >= 3;
}

bool UCombatStateMachine::CheckRunToGround()
{
    // MoveAxis < 0.3 持续 5f
    return MoveAxisLowFrameCount >= 5;
}

bool UCombatStateMachine::CheckAnyToWall()
{
    // bWallDetect==true & Jump
    return bWallDetect && bJumpPressed;
}

bool UCombatStateMachine::CheckWallToAir()
{
    // WallKick 动作播放完 或 Timer 到
    // 简化实现：检查墙壁计时器是否过期
    if (UWorld* World = GetWorld())
    {
        return !World->GetTimerManager().IsTimerActive(WallTimer);
    }
    return false;
}

bool UCombatStateMachine::CheckAnyToDefensive()
{
    // Block 在 ParryWindow 或 WindRun 输入
    return (bBlockPressed && bParryWindow) || bWindRunPressed;
}

bool UCombatStateMachine::CheckDefensiveToGround()
{
    // 防御动作 Recovery 完成
    // 简化实现：防御状态持续一定时间后自动结束
    return StateTime > 1.0f; // 1秒后结束防御状态
}

bool UCombatStateMachine::CheckAirToFSRebound()
{
    // Flying Swallow OnHit 设置
    // TODO: 需要配合战斗系统实现，这里暂时返回false
    return false;
}

bool UCombatStateMachine::CheckFSReboundToAir()
{
    // 18f 后自动退回 Air
    // 由定时器处理，这里检查定时器状态
    if (UWorld* World = GetWorld())
    {
        return !World->GetTimerManager().IsTimerActive(FSReboundTimer);
    }
    return false;
}
