#include "FlyingSwallowScorer.h"
#include "CombatInputBuffer.h"
#include "ComboMoveData.h"

float UFlyingSwallowScorer::CalculateScore_Implementation(const FComboScoreContext& Context) const
{
    if (!Context.InputBufferPtr || Context.MatchedPatternStartIndex < 0)
    {
        return 0.0f;
    }
    const TArray<FCombatInputEntry>& InputBuffer = *Context.InputBufferPtr;

    // 首先调用父类的默认评分
    float Score = Super::CalculateScore_Implementation(Context);

    if (InputBuffer.Num() == 0 || !Context.MoveData || Context.MatchedPatternStartIndex < 0)
    {
        return Score;
    }

    // 飞燕特殊逻辑：检查输入序列中的 Y 轴（向上）输入
    const int32 PatternLength = Context.MoveData->InputPattern.Num();
    const int32 EndIndex = Context.MatchedPatternStartIndex + PatternLength;

    if (EndIndex <= InputBuffer.Num())
    {
        float YAxisBonusAccumulated = 0.0f;
        int32 YInputCount = 0;

        // 遍历匹配的输入模式，检查方向输入
        for (int32 i = Context.MatchedPatternStartIndex; i < EndIndex; ++i)
        {
            const FCombatInputEntry& Entry = InputBuffer[i];
            
            // 检查是否有显著的向上输入（Y > MinYThreshold）
            if (Entry.Direction.Y >= MinYThreshold)
            {
                YAxisBonusAccumulated += YAxisBonus;
                YInputCount++;
            }
        }

        // 如果有多个Y轴输入，给予递减奖励（避免过度奖励）
        if (YInputCount > 1)
        {
            YAxisBonusAccumulated *= FMath::Pow(0.8f, YInputCount - 1); // 每多一个Y输入，奖励递减20%
        }

        Score += YAxisBonusAccumulated;

        // 额外奖励：如果整个序列都有持续的Y轴输入
        if (YInputCount == PatternLength && PatternLength > 1)
        {
            Score += 15.0f; // 连续Y轴输入奖励
        }
    }

    return Score;
}
