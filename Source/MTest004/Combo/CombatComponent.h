// CombatComponent.h
#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CombatInputBuffer.h"          // ECombatInputKey & 输入缓冲
#include "CombatStateMachine.h"         // 状态机与 ECombatState
#include "ComboScorer.h"               // 评分器
#include "ComboMoveData.h"             // 招式数据
#include "GameFramework/Character.h"    // 添加Character支持
#include "Animation/AnimInstance.h"    // 添加动画支持
#include "CombatComponent.generated.h"

// 触发招式事件（AnimBP / FX 可绑定）
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnComboMoveTriggered, const UComboMoveData*, Move);

// 定义日志类别
DECLARE_LOG_CATEGORY_EXTERN(LogCombatComponent, Log, All);

/**
 * 战斗顶层组件：聚合输入缓冲、评分器、状态机
 * 角色仅需把输入转发到 HandleInput()
 */
UCLASS(ClassGroup = (Combat), meta = (BlueprintSpawnableComponent))
class MTEST004_API UCombatComponent : public UActorComponent
{
    GENERATED_BODY()
public:
    UCombatComponent();

    /** 角色输入唯一入口（Character 调用）*/
    UFUNCTION(BlueprintCallable, Category = "Combat")
    void HandleInput(ECombatInputKey Key, FVector2D Dir = FVector2D::ZeroVector);

    /** 当前战斗状态 */
    UFUNCTION(BlueprintPure, Category = "Combat")
    ECombatState GetCurrentState() const;

    /** 招式触发事件 */
    UPROPERTY(BlueprintAssignable, Category = "Combat")
    FOnComboMoveTriggered OnComboMoveTriggered;

    /** 执行招式（播放动画、设置状态等） */
    UFUNCTION(BlueprintCallable, Category = "Combat|Combo")
    void ExecuteComboMove(const UComboMoveData* MoveData);

    /** 连招窗口开启（动画通知调用） */
    UFUNCTION(BlueprintCallable, Category = "Combat|Combo")
    void OnComboWindowOpen();

    /** 连招窗口关闭（动画通知调用） */
    UFUNCTION(BlueprintCallable, Category = "Combat|Combo")
    void OnComboWindowClose();

    /** 招式结束（动画通知调用） */
    UFUNCTION(BlueprintCallable, Category = "Combat|Combo")
    void OnComboMoveEnd();

    /** 检查连招规则 */
    UFUNCTION(BlueprintPure, Category = "Combat|Combo")
    bool IsValidNextMove(const UComboMoveData* CurrentMove, const UComboMoveData* NextMoveCandidate) const;

protected:
    virtual void BeginPlay() override;

    /** 输入 → 匹配 → 评分 → 触发 */
    void EvaluateInput();

    /** 选择最佳招式，考虑当前状态和连招规则 */
    const UComboMoveData* SelectBestMove(int32& OutFlexBits) const;

    /* ---------------- 子系统 ---------------- */
protected:
    /** 必须已存在：可由 Character 创建 */
    UPROPERTY()
    UCombatInputBuffer* InputBuffer = nullptr;

    /** 状态机：可由 Character 或蓝图创建 */
    UPROPERTY()
    UCombatStateMachine* StateMachine = nullptr;

    /** 评分器：可在蓝图替换自定义类 */
    UPROPERTY(EditAnywhere, Instanced, Category = "Combat|Scoring")
    UComboScorer* Scorer = nullptr;

    /** 当前执行中的招式 */
    UPROPERTY(BlueprintReadOnly, Category = "Combat|Combo")
    const UComboMoveData* CurrentComboMove = nullptr;

    /** 是否正在执行招式 */
    UPROPERTY(BlueprintReadOnly, Category = "Combat|Combo")
    bool bIsInComboMove = false;

    /** 连招窗口是否开启 */
    UPROPERTY(BlueprintReadOnly, Category = "Combat|Combo")
    bool bIsComboWindowOpen = false;

    /** 安全定时器句柄 */
    FTimerHandle ComboSafetyTimerHandle;

    /** 缓存拥有者角色 */
    UPROPERTY()
    TWeakObjectPtr<ACharacter> OwnerCharacter;

    /* ---------------- 可调参数 ---------------- */
public:
    /** 可用招式池 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    TArray<TObjectPtr<UComboMoveData>> MoveLibrary;

    /** 触发分数阈值 */
    UPROPERTY(EditDefaultsOnly, Category = "Combat|Scoring")
    float MinTriggerScore = 1.0f;

    /** 若招式要求保留缓冲，保留条数 */
    UPROPERTY(EditDefaultsOnly, Category = "Combat|Input")
    int32 PreserveCountIfRequested = 1;
};