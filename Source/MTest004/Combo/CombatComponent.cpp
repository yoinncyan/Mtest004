// CombatComponent.cpp
#include "CombatComponent.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Character.h"
#include "Animation/AnimInstance.h"
#include "TimerManager.h"

// 定义日志类别
DEFINE_LOG_CATEGORY(LogCombatComponent);

UCombatComponent::UCombatComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UCombatComponent::BeginPlay()
{
    Super::BeginPlay();

    AActor* Owner = GetOwner();
    if (!Owner)
    {
        UE_LOG(LogCombatComponent, Error, TEXT("UCombatComponent::BeginPlay - Invalid Owner"));
        return;
    }

    // 尝试查找现有组件
    InputBuffer = Owner->FindComponentByClass<UCombatInputBuffer>();
    StateMachine = Owner->FindComponentByClass<UCombatStateMachine>();

    // 若角色未挂 InputBuffer，自动创建
    if (!InputBuffer)
    {
        InputBuffer = NewObject<UCombatInputBuffer>(Owner, UCombatInputBuffer::StaticClass(), TEXT("AutoInputBuffer"));
        InputBuffer->RegisterComponent();
    }

    // 若未挂状态机，也自动创建（可按需在蓝图中替换）
    if (!StateMachine)
    {
        StateMachine = NewObject<UCombatStateMachine>(Owner, UCombatStateMachine::StaticClass(), TEXT("AutoStateMachine"));
        StateMachine->RegisterComponent();
        StateMachine->InitializeTransitionRules(); // 确保状态机规则初始化
    }

    // 默认评分器
    if (!Scorer)
    {
        Scorer = NewObject<UComboScorer>(this, UComboScorer::StaticClass());
    }

    // 缓存角色引用
    OwnerCharacter = Cast<ACharacter>(Owner);
    if (!OwnerCharacter.IsValid())
    {
        UE_LOG(LogCombatComponent, Warning, TEXT("UCombatComponent::BeginPlay - Owner is not a Character"));
    }
}

// ────────────────────────────────────────────────────────────────────────────────
void UCombatComponent::HandleInput(ECombatInputKey Key, FVector2D Dir)
{
    // 基本验证
    if (!InputBuffer) return;
    if (Key >= ECombatInputKey::CIK_MAX)
    {
        UE_LOG(LogCombatComponent, Warning, TEXT("Invalid input key: %d"), (int32)Key);
        return;
    }

    // 方向输入归一化
    if (!Dir.IsNearlyZero())
    {
        Dir.Normalize();
    }

    // 记录输入并评估
    InputBuffer->ProcessInput(Key, Dir);
    EvaluateInput();
}

ECombatState UCombatComponent::GetCurrentState() const
{
    return StateMachine ? StateMachine->GetCurrentState() : ECombatState::CS_Idle;
}

// ────────────────────────────────────────────────────────────────────────────────
void UCombatComponent::EvaluateInput()
{
    if (!InputBuffer || !Scorer)
    {
        UE_LOG(LogCombatComponent, Warning, TEXT("EvaluateInput: Missing InputBuffer or Scorer"));
        return;
    }

    // 如果当前不在连招窗口且正在执行招式，不处理新输入
    if (bIsInComboMove && !bIsComboWindowOpen)
    {
        UE_LOG(LogCombatComponent, VeryVerbose, TEXT("不在连招窗口，忽略输入"));
        return;
    }

    int32 FlexBitsUsed = 0;
    const UComboMoveData* BestMove = SelectBestMove(FlexBitsUsed);
    if (!BestMove)
    {
        UE_LOG(LogCombatComponent, VeryVerbose, TEXT("未找到匹配招式"));
        return;
    }

    // 执行找到的最佳招式
    ExecuteComboMove(BestMove);

    // 根据招式要求处理缓冲
    if (BestMove->PreserveBuffer)
    {
        InputBuffer->PreserveRecentInputs(PreserveCountIfRequested);
        UE_LOG(LogCombatComponent, Verbose, TEXT("保留最近%d个输入"), PreserveCountIfRequested);
    }
    else
    {
        InputBuffer->ClearBuffer();
        UE_LOG(LogCombatComponent, Verbose, TEXT("清空输入缓冲"));
    }
}

// ────────────────────────────────────────────────────────────────────────────────
const UComboMoveData* UCombatComponent::SelectBestMove(int32& OutFlexBits) const
{
    const float Now = GetWorld()->GetTimeSeconds();
    float BestScore = -1.f;
    const UComboMoveData* BestMove = nullptr;

    // 获取当前战斗状态，用于过滤招式
    ECombatState CurrentState = GetCurrentState();
    UE_LOG(LogCombatComponent, Verbose, TEXT("当前战斗状态: %s"), *UEnum::GetValueAsString(CurrentState));

    for (const UComboMoveData* Move : MoveLibrary)
    {
        if (!Move) continue;

        // 检查招式是否适用于当前状态
        if (!Move->ValidStates.Contains(CurrentState))
        {
            UE_LOG(LogCombatComponent, VeryVerbose, TEXT("招式 %s 不适用于当前状态"), *Move->MoveID.ToString());
            continue;
        }

        // 检查连招规则
        if (CurrentComboMove && !IsValidNextMove(CurrentComboMove, Move))
        {
            UE_LOG(LogCombatComponent, VeryVerbose, TEXT("招式 %s 不符合连招规则"), *Move->MoveID.ToString());
            continue;
        }

        int32 FlexUsed = 0;
        int32 StartIdx = InputBuffer->FindBestMatchStartIndex(Move, Move->InputFlexBits, &FlexUsed);
        if (StartIdx < 0) continue; // 无匹配

        // 获取过滤后的缓冲区用于评分
        auto FilteredBuffer = InputBuffer->GetFilteredBuffer();

        FComboScoreContext Ctx;
        Ctx.InputBufferPtr = &FilteredBuffer;  // 正确设置输入缓冲区指针
        Ctx.MoveData = Move;
        Ctx.BaseScore = Move->BaseScore;
        Ctx.MaxTimeWindow = 1.0f;
        Ctx.MatchedPatternStartIndex = StartIdx;
        Ctx.FlexBitsUsed = FlexUsed;
        Ctx.PatternLength = Move->InputPattern.Num();
        Ctx.DefaultInputLifespan = 0.8f; // 使用合理的默认值
        Ctx.CurrentGameTime = Now;

        const float Score = Scorer->CalculateScore(Ctx);
        UE_LOG(LogCombatComponent, VeryVerbose, TEXT("招式 %s 评分: %.2f"), *Move->MoveID.ToString(), Score);

        if (Score > BestScore)
        {
            BestScore = Score;
            BestMove = Move;
            OutFlexBits = FlexUsed;
        }
    }

    if (BestMove)
    {
        UE_LOG(LogCombatComponent, Verbose, TEXT("选择招式 %s，得分: %.2f (阈值: %.2f)"),
            *BestMove->MoveID.ToString(), BestScore, MinTriggerScore);
    }

    return (BestScore >= MinTriggerScore) ? BestMove : nullptr;
}

// ────────────────────────────────────────────────────────────────────────────────
// 招式执行与生命周期管理
// ────────────────────────────────────────────────────────────────────────────────

void UCombatComponent::ExecuteComboMove(const UComboMoveData* MoveData)
{
    if (!MoveData)
    {
        UE_LOG(LogCombatComponent, Error, TEXT("尝试执行空招式"));
        return;
    }

    // 设置当前招式状态
    CurrentComboMove = MoveData;
    bIsInComboMove = true;
    bIsComboWindowOpen = false;

    UE_LOG(LogCombatComponent, Log, TEXT("执行招式: %s"), *MoveData->MoveID.ToString());

    // 播放招式动画
    if (OwnerCharacter.IsValid() && MoveData->AttackMontage)
    {
        UAnimInstance* AnimInstance = OwnerCharacter->GetMesh()->GetAnimInstance();
        if (AnimInstance)
        {
            float MontageLength = AnimInstance->Montage_Play(MoveData->AttackMontage);

            UE_LOG(LogCombatComponent, Verbose, TEXT("播放动画: %s (持续: %.2f)"),
                *MoveData->AttackMontage->GetName(), MontageLength);

            // 如果动画播放失败，设置安全定时器以确保系统不会卡住
            if (MontageLength <= 0.0f)
            {
                GetWorld()->GetTimerManager().SetTimer(
                    ComboSafetyTimerHandle,
                    this,
                    &UCombatComponent::OnComboMoveEnd,
                    0.1f,
                    false
                );
                UE_LOG(LogCombatComponent, Warning, TEXT("动画播放失败，启用安全定时器"));
            }
        }
        else
        {
            UE_LOG(LogCombatComponent, Warning, TEXT("未找到AnimInstance，无法播放动画"));
        }
    }

    // 根据招式选择合适的状态转换目标和优先级
    if (StateMachine)
    {
        // 通常战斗招式应切换到"Ground"状态（示例）
        ECombatState TargetState = ECombatState::CS_Ground;
        int32 Priority = 2;  // 默认优先级

        StateMachine->RequestStateTransition(TargetState, Priority);
    }

    // 广播事件
    OnComboMoveTriggered.Broadcast(MoveData);
}

void UCombatComponent::OnComboWindowOpen()
{
    if (!bIsInComboMove) return;

    bIsComboWindowOpen = true;
    UE_LOG(LogCombatComponent, Verbose, TEXT("连招窗口开启: %s"),
        CurrentComboMove ? *CurrentComboMove->MoveID.ToString() : TEXT("Unknown"));
}

void UCombatComponent::OnComboWindowClose()
{
    if (!bIsInComboMove) return;

    bIsComboWindowOpen = false;
    UE_LOG(LogCombatComponent, Verbose, TEXT("连招窗口关闭: %s"),
        CurrentComboMove ? *CurrentComboMove->MoveID.ToString() : TEXT("Unknown"));
}

void UCombatComponent::OnComboMoveEnd()
{
    if (!bIsInComboMove) return;

    UE_LOG(LogCombatComponent, Verbose, TEXT("招式结束: %s"),
        CurrentComboMove ? *CurrentComboMove->MoveID.ToString() : TEXT("Unknown"));

    bIsInComboMove = false;
    bIsComboWindowOpen = false;

    // 清除安全定时器
    if (GetWorld()->GetTimerManager().IsTimerActive(ComboSafetyTimerHandle))
    {
        GetWorld()->GetTimerManager().ClearTimer(ComboSafetyTimerHandle);
    }

    CurrentComboMove = nullptr;

    // 通常在招式结束时重置为基本状态（如果没有其他状态干预）
    // 这里使用较低优先级，以便其他高优先级状态转换可以覆盖
    if (StateMachine)
    {
        StateMachine->RequestStateTransition(ECombatState::CS_Ground, 1);
    }
}

bool UCombatComponent::IsValidNextMove(const UComboMoveData* CurrentMove, const UComboMoveData* NextMoveCandidate) const
{
    // 如果没有当前招式或不在连招窗口，只能使用基础招式
    if (!CurrentMove || !bIsComboWindowOpen)
    {
        // 这里可以添加识别初始招式的逻辑
        // 示例: return NextMoveCandidate->bIsBaseMove;
        return true; // 简化实现，允许任何招式作为起始招式
    }

    // 如果NextMoves为空，允许任何招式
    if (CurrentMove->NextMoves.Num() == 0)
    {
        return true;
    }

    // 检查NextMoves列表
    for (const TWeakObjectPtr<const UComboMoveData>& NextMove : CurrentMove->NextMoves)
    {
        if (NextMove.IsValid() && NextMove.Get() == NextMoveCandidate)
        {
            return true;
        }
    }

    return false;
}