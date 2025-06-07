#include "CombatStateMachineExample.h"
#include "Engine/World.h"
#include "TimerManager.h"

UCombatStateMachineExample::UCombatStateMachineExample()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PrePhysics;
}

void UCombatStateMachineExample::BeginPlay()
{
    Super::BeginPlay();
    
    // 创建状态机实例
    StateMachine = NewObject<UCombatStateMachine>(this, TEXT("ExampleStateMachine"));
    
    if (StateMachine)
    {
        UE_LOG(LogTemp, Log, TEXT("Combat State Machine Example: State machine initialized successfully"));
    }
}

void UCombatStateMachineExample::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (StateMachine)
    {
        // 更新状态机输入状态
        StateMachine->SetInputPressed(bJumpPressed, bBlockPressed, bWindRunPressed);
        StateMachine->SetMovementState(CurrentMoveAxis, bOnFloor, bWallDetect);
        StateMachine->SetCombatFlags(bParryWindow, false); // bIsAirborne 由状态机内部管理
        
        // 更新状态机
        StateMachine->UpdateState(DeltaTime);
    }
}

void UCombatStateMachineExample::TriggerJump()
{
    bJumpPressed = true;
    UE_LOG(LogTemp, Log, TEXT("State Machine Example: Jump triggered"));
    
    // 自动重置输入（模拟按键释放）
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().SetTimer(InputResetTimer, 
            [this]() { bJumpPressed = false; }, 
            0.1f, false);
    }
}

void UCombatStateMachineExample::TriggerBlock()
{
    bBlockPressed = true;
    bParryWindow = true; // 模拟格挡窗口
    UE_LOG(LogTemp, Log, TEXT("State Machine Example: Block triggered"));
    
    // 自动重置输入和格挡窗口
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().SetTimer(InputResetTimer, 
            [this]() { 
                bBlockPressed = false; 
                bParryWindow = false;
            }, 
            0.5f, false);
    }
}

void UCombatStateMachineExample::TriggerRun(bool bStartRunning)
{
    CurrentMoveAxis = bStartRunning ? 1.0f : 0.0f;
    UE_LOG(LogTemp, Log, TEXT("State Machine Example: Run %s"), bStartRunning ? TEXT("started") : TEXT("stopped"));
}

void UCombatStateMachineExample::SetOnGround(bool bOnGround)
{
    bOnFloor = bOnGround;
    UE_LOG(LogTemp, Log, TEXT("State Machine Example: On ground set to %s"), bOnGround ? TEXT("true") : TEXT("false"));
}

void UCombatStateMachineExample::SetWallDetection(bool bDetectWall)
{
    bWallDetect = bDetectWall;
    UE_LOG(LogTemp, Log, TEXT("State Machine Example: Wall detection set to %s"), bDetectWall ? TEXT("true") : TEXT("false"));
}

ECombatState UCombatStateMachineExample::GetCurrentState() const
{
    if (StateMachine)
    {
        return StateMachine->GetCurrentState();
    }
    return ECombatState::CS_Idle;
}

FString UCombatStateMachineExample::GetCurrentStateAsString() const
{
    ECombatState CurrentState = GetCurrentState();
    
    switch (CurrentState)
    {
        case ECombatState::CS_Idle: return TEXT("Idle");
        case ECombatState::CS_Ground: return TEXT("Ground");
        case ECombatState::CS_Run: return TEXT("Run");
        case ECombatState::CS_Air: return TEXT("Air");
        case ECombatState::CS_Wall: return TEXT("Wall");
        case ECombatState::CS_Defensive: return TEXT("Defensive");
        case ECombatState::CS_UTCharge: return TEXT("UT Charge");
        case ECombatState::CS_FS_Rebound: return TEXT("Flying Swallow Rebound");
        case ECombatState::CS_WaterRun: return TEXT("Water Run");
        default: return TEXT("Unknown");
    }
}
