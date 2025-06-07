#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "ComboScorerUsageExample.generated.h"

/**
 * 连招评分系统使用示例和说明
 * 这个类主要用于展示如何使用评分系统，不是核心功能类
 */
UCLASS()
class MTEST004_API UComboScorerUsageExample : public UObject
{
    GENERATED_BODY()

public:
    /**
     * 展示如何使用连招评分系统的示例函数
     */
    UFUNCTION(BlueprintCallable, Category = "Combo Scorer Example")
    void ExampleUsage();
};
