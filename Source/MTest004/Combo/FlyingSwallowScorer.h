#pragma once

#include "CoreMinimal.h"
#include "ComboScorer.h"
#include "FlyingSwallowScorer.generated.h"

/**
 * 飞燕特殊评分器示例
 * 提高 Y 轴（向上）输入的权重，符合飞燕招式的特性
 */
UCLASS(BlueprintType)
class MTEST004_API UFlyingSwallowScorer : public UComboScorer
{
    GENERATED_BODY()

public:
    virtual float CalculateScore_Implementation(const FComboScoreContext& Context) const override;

protected:
    // 可在蓝图中配置的参数
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Flying Swallow")
    float YAxisBonus = 25.0f; // Y轴输入的额外奖励

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Flying Swallow")
    float MinYThreshold = 0.7f; // Y轴输入的最小阈值
};
