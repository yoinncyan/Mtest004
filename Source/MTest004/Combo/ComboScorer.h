#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "UObject/Object.h"
#include "ComboScorer.generated.h"

// Forward declarations
struct FCombatInputEntry;
class UComboMoveData;

/**
 * Context for calculating a combo move's score.
 * This can be expanded with more parameters as needed.
 */
USTRUCT(BlueprintType)
struct FComboScoreContext
{
    GENERATED_BODY()

    // 移除指针，改用值类型
    //UPROPERTY(BlueprintReadWrite, Category = "Scoring")
    //TArray<FCombatInputEntry> InputBuffer;    // 使用const指针与其他组件保持一致，不能用UPROPERTY修饰const指针

    const TArray<FCombatInputEntry>* InputBufferPtr = nullptr; // 使用指针引用外部缓冲区

    UPROPERTY(BlueprintReadWrite, Category = "Scoring")
    const UComboMoveData* MoveData = nullptr;

    UPROPERTY(BlueprintReadWrite, Category = "Scoring")
    float BaseScore = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Scoring")
    float MaxTimeWindow = 1.0f; // Max time allowed for the pattern, for Δt penalty calculation

    UPROPERTY(BlueprintReadWrite, Category = "Scoring")
    int32 MatchedPatternStartIndex = -1; // Where in the buffer the pattern starts

    UPROPERTY(BlueprintReadWrite, Category = "Scoring")
    int32 FlexBitsUsed = 0; // Bitmask of which inputs used flexibility

    UPROPERTY(BlueprintReadWrite, Category = "Scoring")
    int32 PatternLength = 0; // Length of the matched pattern
    
    UPROPERTY(BlueprintReadWrite, Category = "Scoring")
    float DefaultInputLifespan = 0.8f; // 输入默认有效期，供评分使用
    
    UPROPERTY(BlueprintReadWrite, Category = "Scoring")
    float CurrentGameTime = 0.0f; // 当前游戏时间，用于评估输入新鲜度
};

UCLASS(Blueprintable)
class MTEST004_API UComboScorer : public UObject
{
    GENERATED_BODY()

public:
    /**
     * Calculates the score for a given move based on the input context.
     * Formula elements: Base + OrderBonus – Δt Penalty + FlexBonus + FreshnessBonus + RhythmBonus
     * @param Context The scoring context containing input buffer, move data, etc.
     * @return The calculated score. Higher is better.
     */
    UFUNCTION(BlueprintNativeEvent, Category = "Scoring")
    float CalculateScore(const FComboScoreContext& Context) const;
    virtual float CalculateScore_Implementation(const FComboScoreContext& Context) const;

    // 提供蓝图可见的接口方法
    UFUNCTION(BlueprintPure, Category = "Scoring")
    static TArray<FCombatInputEntry> GetInputBufferFromContext(const FComboScoreContext& Context)
    {
        if (Context.InputBufferPtr) 
            return *Context.InputBufferPtr;
        return TArray<FCombatInputEntry>();
    }
    
protected:
    /** 计算输入新鲜度分数 - 越新的输入，分数越高 */
    UFUNCTION(BlueprintCallable, Category = "Scoring")
    float CalculateFreshnessBonus(const FComboScoreContext& Context) const;
    
    /** 计算输入节奏一致性分数 - 评估输入间隔的一致性 */
    UFUNCTION(BlueprintCallable, Category = "Scoring")
    float CalculateRhythmBonus(const FComboScoreContext& Context) const;
    
    /** 计算最近输入奖励 - 奖励最近输入的匹配 */
    UFUNCTION(BlueprintCallable, Category = "Scoring")
    float CalculateRecentInputBonus(const FComboScoreContext& Context) const;
};