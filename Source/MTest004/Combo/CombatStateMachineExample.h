#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CombatStateMachine.h"
#include "CombatStateMachineExample.generated.h"

/**
 * 战斗状态机使用示例组件
 * 展示如何正确配置和使用战斗状态机系统
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class MTEST004_API UCombatStateMachineExample : public UActorComponent
{
    GENERATED_BODY()

public:    
    UCombatStateMachineExample();

protected:
    virtual void BeginPlay() override;

public:    
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // 示例函数：展示如何手动触发状态转换
    UFUNCTION(BlueprintCallable, Category = "State Machine Example")
    void TriggerJump();
    
    UFUNCTION(BlueprintCallable, Category = "State Machine Example")
    void TriggerBlock();
    
    UFUNCTION(BlueprintCallable, Category = "State Machine Example")
    void TriggerRun(bool bStartRunning);
    
    UFUNCTION(BlueprintCallable, Category = "State Machine Example")
    void SetOnGround(bool bOnGround);
    
    UFUNCTION(BlueprintCallable, Category = "State Machine Example")
    void SetWallDetection(bool bDetectWall);

    // 获取当前状态（用于调试）
    UFUNCTION(BlueprintPure, Category = "State Machine Example")
    ECombatState GetCurrentState() const;
    
    UFUNCTION(BlueprintPure, Category = "State Machine Example")
    FString GetCurrentStateAsString() const;

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State Machine")
    UCombatStateMachine* StateMachine;
    
    // 模拟输入状态
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Input Simulation")
    bool bJumpPressed = false;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Input Simulation")
    bool bBlockPressed = false;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Input Simulation")
    bool bWindRunPressed = false;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement Simulation")
    float CurrentMoveAxis = 0.0f;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement Simulation")
    bool bOnFloor = true;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement Simulation")
    bool bWallDetect = false;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat Simulation")
    bool bParryWindow = false;
    
    // 自动输入模拟定时器
    FTimerHandle InputResetTimer;
};
