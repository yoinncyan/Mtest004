#include "ComboScorer.h"
#include "CombatInputBuffer.h" // For FCombatInputEntry
#include "ComboMoveData.h"     // For UComboMoveData

// Default implementation for CalculateScore
// Score = Base + OrderBonus – Δt Penalty + FlexBonus + FreshnessBonus + RhythmBonus

float UComboScorer::CalculateScore_Implementation(const FComboScoreContext& Context) const
{
    // 检查指针有效性
    if (!Context.InputBufferPtr || !Context.MoveData || Context.MatchedPatternStartIndex < 0)
    {
        return 0.0f; // 无效上下文
    }
        // 确保CurrentGameTime字段有效
    if (Context.CurrentGameTime <= 0.0f)
    {
        UE_LOG(LogTemp, Warning, TEXT("警告: ComboScorer的CurrentGameTime无效值: %f"), Context.CurrentGameTime);
        return 0.0f;
    }

    // 缓存常用值，避免重复访问和边界检查
    const TArray<FCombatInputEntry>& InputBuffer = *Context.InputBufferPtr;
    const int32 BufferSize = InputBuffer.Num();
    const int32 MatchStartIdx = Context.MatchedPatternStartIndex;
    
    if (BufferSize == 0 || !Context.MoveData || MatchStartIdx < 0)
    {
        return 0.0f; // 无效上下文
    }

    const int32 PatternLength = Context.MoveData->InputPattern.Num();
    const int32 EndIndex = FMath::Min(MatchStartIdx + PatternLength, BufferSize);
    
    // 提前检查是否有足够的输入来匹配模式
    if (PatternLength == 0 || EndIndex - MatchStartIdx < PatternLength)
    {
        return 0.0f;
    }

    // 缓存数组引用，避免重复解析
    //const TArray<FCombatInputEntry>& InputBuffer = Context.InputBuffer;
    float Score = Context.BaseScore;
    
    // --- 输入新鲜度评分 (FreshnessBonus) ---
    Score += CalculateFreshnessBonus(Context);
    
    // --- 匹配位置评分 (OrderBonus) ---
    if (EndIndex == BufferSize) {
        Score += 50.0f; // 匹配最新输入时的奖励分数
    } else {
        // 距离末尾越远，分数越低，但惩罚减轻（更宽容）
        Score -= (BufferSize - EndIndex) * 3.0f;
    }

    // --- 时间窗口评分 (Δt Penalty) ---
    if (PatternLength > 0 && EndIndex - MatchStartIdx == PatternLength)
    {
        const float FirstInputTime = InputBuffer[MatchStartIdx].Timestamp;
        const float LastInputTime = InputBuffer[EndIndex - 1].Timestamp;
        const float TimeTaken = LastInputTime - FirstInputTime;
        
        // 更宽容的时间窗口评分
        if (TimeTaken > Context.MaxTimeWindow)
        {
            // 应用惩罚，但采用非线性曲线使惩罚更平滑
            const float OverTimeRatio = TimeTaken / Context.MaxTimeWindow;
            Score -= 40.0f * FMath::Log2(OverTimeRatio); // 对超时惩罚采用对数缩放
        }
        else
        {
            // 奖励在合理时间窗口内完成的输入序列
            const float TimeEfficiency = 1.0f - (TimeTaken / Context.MaxTimeWindow);
            Score += 30.0f * TimeEfficiency; // 最多30分的效率奖励
        }
    }
    
    // --- 弹性输入奖励 (FlexBonus) ---
    if (Context.FlexBitsUsed > 0)
    {
        // 计算FlexBitsUsed中设置的位数
        int32 FlexInputCount = 0;
        int32 TempFlexBits = Context.FlexBitsUsed;
        
        // 使用更高效的位计数方法
        // 等效于 FMath::CountBits(Context.FlexBitsUsed)
        while (TempFlexBits > 0)
        {
            FlexInputCount += (TempFlexBits & 1);
            TempFlexBits >>= 1;
        }
        
        Score += FlexInputCount * 5.0f; // 每个弹性输入5分奖励
    }
    
    // --- 最近输入奖励 (RecentInputBonus) ---
    Score += CalculateRecentInputBonus(Context);
    
    // --- 节奏一致性评分 (RhythmBonus) ---
    Score += CalculateRhythmBonus(Context);

    return FMath::Max(0.0f, Score); // 确保分数不为负
}

// float UComboScorer::CalculateScore_Implementation(const FComboScoreContext& Context) const
// {
//     if (Context.InputBuffer.Num() == 0 || !Context.MoveData || Context.MatchedPatternStartIndex < 0)
//     {
//         return 0.0f; // Invalid context
//     }

//     float Score = Context.BaseScore;
    
//     // --- 输入新鲜度评分 (FreshnessBonus) ---
//     Score += CalculateFreshnessBonus(Context);
    
//     // --- 匹配位置评分 (OrderBonus) ---
//     if (Context.InputBuffer.Num() > 0) {
//         // 如果模式在缓冲区末尾匹配（最新输入），给予更高分数
//         int32 EndPosition = Context.MatchedPatternStartIndex + Context.MoveData->InputPattern.Num();
//         if (EndPosition == Context.InputBuffer.Num()) {
//             Score += 50.0f; // 匹配最新输入时的奖励分数
//         } else {
//             // 距离末尾越远，分数越低，但惩罚减轻（更宽容）
//             Score -= (Context.InputBuffer.Num() - EndPosition) * 3.0f;
//         }
//     }

//     // --- 时间窗口评分 (Δt Penalty) ---
//     if (Context.MoveData->InputPattern.Num() > 0 && 
//         Context.MatchedPatternStartIndex + Context.MoveData->InputPattern.Num() <= Context.InputBuffer.Num())
//     {
//         const float FirstInputTime = Context.InputBuffer[Context.MatchedPatternStartIndex].Timestamp;
//         const float LastInputTime = Context.InputBuffer[Context.MatchedPatternStartIndex + Context.MoveData->InputPattern.Num() - 1].Timestamp;
//         const float TimeTaken = LastInputTime - FirstInputTime;
        
//         // 更宽容的时间窗口评分
//         if (TimeTaken > Context.MaxTimeWindow)
//         {
//             // 应用惩罚，但采用非线性曲线使惩罚更平滑
//             float OverTimeRatio = TimeTaken / Context.MaxTimeWindow;
//             Score -= 40.0f * FMath::Log2(OverTimeRatio); // 对超时惩罚采用对数缩放
//         }
//         else
//         {
//             // 奖励在合理时间窗口内完成的输入序列
//             float TimeEfficiency = 1.0f - (TimeTaken / Context.MaxTimeWindow);
//             Score += 30.0f * TimeEfficiency; // 最多30分的效率奖励
//         }
//     }
    
//     // --- 弹性输入奖励 (FlexBonus) ---
//     if (Context.FlexBitsUsed > 0)
//     {
//         // 计算FlexBitsUsed中设置的位数
//         int32 FlexInputCount = 0;
//         int32 TempFlexBits = Context.FlexBitsUsed;
//         while (TempFlexBits > 0)
//         {
//             if (TempFlexBits & 1) FlexInputCount++;
//             TempFlexBits >>= 1;
//         }
//         Score += FlexInputCount * 5.0f; // 每个弹性输入5分奖励
//     }
    
//     // --- 最近输入奖励 (RecentInputBonus) ---
//     Score += CalculateRecentInputBonus(Context);
    
//     // --- 节奏一致性评分 (RhythmBonus) ---
//     Score += CalculateRhythmBonus(Context);

//     return FMath::Max(0.0f, Score); // 确保分数不为负
// }

// float UComboScorer::CalculateFreshnessBonus(const FComboScoreContext& Context) const
// {
//     if (Context.InputBuffer.Num() == 0 || Context.MatchedPatternStartIndex < 0)
//     {
//         return 0.0f;
//     }
    
//     // 使用当前时间或传入的时间
//     float CurrentTime = Context.CurrentGameTime > 0.0f ? 
//                         Context.CurrentGameTime : 
//                         FPlatformTime::Seconds();
    
//     // 计算匹配的输入序列的平均新鲜度
//     float TotalFreshness = 0.0f;
//     int32 EndIndex = Context.MatchedPatternStartIndex + Context.MoveData->InputPattern.Num();
//     EndIndex = FMath::Min(EndIndex, Context.InputBuffer.Num());
    
//     for (int32 i = Context.MatchedPatternStartIndex; i < EndIndex; i++)
//     {
//         float InputAge = CurrentTime - Context.InputBuffer[i].Timestamp;
//         float FreshnessFactor = FMath::Clamp(1.0f - (InputAge / Context.DefaultInputLifespan), 0.0f, 1.0f);
//         TotalFreshness += FreshnessFactor;
//     }
    
//     float AverageFreshness = TotalFreshness / (EndIndex - Context.MatchedPatternStartIndex);
    
//     // 新鲜度奖励：0-40分，随着输入的新鲜度增加而增加
//     return 40.0f * AverageFreshness;
// }

// float UComboScorer::CalculateRhythmBonus(const FComboScoreContext& Context) const
// {
//     if (!Context.InputBufferPtr || Context.MatchedPatternStartIndex < 0)
//     {
//         return 0.0f;
//     }
    
//     const TArray<FCombatInputEntry>& InputBuffer = *Context.InputBufferPtr;
//     if (InputBuffer.Num() <= 2 || Context.MatchedPatternStartIndex < 0)
//     {
//         return 0.0f; // 至少需要三个输入才能评估节奏
//     }
    
//     int32 EndIndex = Context.MatchedPatternStartIndex + Context.MoveData->InputPattern.Num();
//     EndIndex = FMath::Min(EndIndex, InputBuffer.Num());
    
//     if (EndIndex - Context.MatchedPatternStartIndex < 3)
//     {
//         return 0.0f; // 至少需要三个输入
//     }
    
//     // 计算输入之间的时间间隔
//     TArray<float> Intervals;
//     for (int32 i = Context.MatchedPatternStartIndex + 1; i < EndIndex; i++)
//     {
//         float Interval = InputBuffer[i].Timestamp - InputBuffer[i-1].Timestamp;
//         Intervals.Add(Interval);
//     }
    
//     // 计算平均间隔
//     float AvgInterval = 0.0f;
//     for (float Interval : Intervals)
//     {
//         AvgInterval += Interval;
//     }
//     AvgInterval /= Intervals.Num();
    
//     // 计算间隔的标准差（节奏一致性）
//     float VarianceSum = 0.0f;
//     for (float Interval : Intervals)
//     {
//         float Deviation = Interval - AvgInterval;
//         VarianceSum += Deviation * Deviation;
//     }
//     float StdDev = FMath::Sqrt(VarianceSum / Intervals.Num());
    
//     // 节奏一致性越高（标准差越小），奖励越高
//     // 但我们不希望惩罚太严重，所以使用非线性映射
//     float ConsistencyFactor = FMath::Clamp(1.0f - (StdDev / (AvgInterval * 0.5f)), 0.0f, 1.0f);
    
//     // 节奏一致性奖励：0-25分
//     return 25.0f * ConsistencyFactor;
// }

float UComboScorer::CalculateRhythmBonus(const FComboScoreContext& Context) const
{
    if (!Context.InputBufferPtr || Context.MatchedPatternStartIndex < 0)
    {
        return 0.0f;
    }
    
    const TArray<FCombatInputEntry>& InputBuffer = *Context.InputBufferPtr;
    if (InputBuffer.Num() <= 2 || Context.MatchedPatternStartIndex < 0)
    {
        return 0.0f; // 至少需要三个输入才能评估节奏
    }
    
    int32 EndIndex = Context.MatchedPatternStartIndex + Context.MoveData->InputPattern.Num();
    EndIndex = FMath::Min(EndIndex, InputBuffer.Num());
    
    if (EndIndex - Context.MatchedPatternStartIndex < 3)
    {
        return 0.0f; // 至少需要三个输入
    }
    
    // 计算输入之间的时间间隔
    TArray<float> Intervals;
    for (int32 i = Context.MatchedPatternStartIndex + 1; i < EndIndex; i++)
    {
        float Interval = InputBuffer[i].Timestamp - InputBuffer[i-1].Timestamp;
        Intervals.Add(Interval);
    }
    
    // 如果只有0或1个间隔，无法计算标准差
    if (Intervals.Num() <= 1)
    {
        return 0.0f; // 无法评估节奏一致性
    }
    
    // 计算平均间隔
    float AvgInterval = 0.0f;
    for (float Interval : Intervals)
    {
        AvgInterval += Interval;
    }
    AvgInterval /= Intervals.Num();
    
    // 计算间隔的标准差（节奏一致性）
    float VarianceSum = 0.0f;
    for (float Interval : Intervals)
    {
        float Deviation = Interval - AvgInterval;
        VarianceSum += Deviation * Deviation;
    }
    
    // 修复：确保分母大于0
    float StdDev = 0.0f;
    if (Intervals.Num() > 1) // 添加此保护条件
    {
        StdDev = FMath::Sqrt(VarianceSum / Intervals.Num());
    }
    
    // 节奏一致性越高（标准差越小），奖励越高
    // 但我们不希望惩罚太严重，所以使用非线性映射
    float ConsistencyFactor = FMath::Clamp(1.0f - (StdDev / (AvgInterval * 0.5f)), 0.0f, 1.0f);
    
    // 节奏一致性奖励：0-25分
    return 25.0f * ConsistencyFactor;
}

// float UComboScorer::CalculateRecentInputBonus(const FComboScoreContext& Context) const
// {
//     if (Context.InputBuffer.Num() == 0 || Context.MatchedPatternStartIndex < 0)
//     {
//         return 0.0f;
//     }
    
//     float CurrentTime = Context.CurrentGameTime > 0.0f ? 
//                         Context.CurrentGameTime : 
//                         FPlatformTime::Seconds();
    
//     // 找出最近的输入时间戳
//     float MostRecentTimestamp = 0.0f;
//     for (int32 i = Context.MatchedPatternStartIndex; 
//          i < Context.MatchedPatternStartIndex + Context.MoveData->InputPattern.Num() && i < Context.InputBuffer.Num(); 
//          i++)
//     {
//         MostRecentTimestamp = FMath::Max(MostRecentTimestamp, Context.InputBuffer[i].Timestamp);
//     }
    
//     // 计算最近输入的年龄
//     float MostRecentAge = CurrentTime - MostRecentTimestamp;
    
//     // 最近输入奖励：如果最近的输入很新（<0.3秒），给予额外奖励
//     // 使用平滑的线性插值而不是硬阈值
//     float RecentThreshold = 0.3f; // 300ms阈值
//     if (MostRecentAge < RecentThreshold)
//     {
//         float RecencyFactor = 1.0f - (MostRecentAge / RecentThreshold);
//         return 35.0f * RecencyFactor; // 最多35分的近期输入奖励
//     }
    
//     return 0.0f;
// }

// float UComboScorer::CalculateFreshnessBonus(const FComboScoreContext& Context) const
// {

//     if (!Context.InputBufferPtr || Context.MatchedPatternStartIndex < 0)
//     {
//         return 0.0f;
//     }
//     const TArray<FCombatInputEntry>& InputBuffer = *Context.InputBufferPtr;
//     const int32 MatchStartIdx = Context.MatchedPatternStartIndex;
//     if (InputBuffer.Num() == 0 || MatchStartIdx < 0)
//     {
//         return 0.0f;
//     }
    
//     // 缓存常用值
//     //const TArray<FCombatInputEntry>& InputBuffer = Context.InputBuffer;
//     const int32 PatternLength = Context.MoveData->InputPattern.Num();
//     const int32 BufferSize = InputBuffer.Num();
//     const int32 EndIndex = FMath::Min(MatchStartIdx + PatternLength, BufferSize);
    
//     if (EndIndex <= MatchStartIdx) return 0.0f;
    
//     // 使用当前时间或传入的时间
//     const float CurrentTime = Context.CurrentGameTime > 0.0f ? 
//                         Context.CurrentGameTime : 
//                         FPlatformTime::Seconds();
    
//     // 计算匹配的输入序列的平均新鲜度
//     float TotalFreshness = 0.0f;
//     const float InvLifespan = 1.0f / Context.DefaultInputLifespan; // 避免除法运算
    
//     for (int32 i = MatchStartIdx; i < EndIndex; i++)
//     {
//         const float InputAge = CurrentTime - InputBuffer[i].Timestamp;
//         const float FreshnessFactor = FMath::Clamp(1.0f - (InputAge * InvLifespan), 0.0f, 1.0f);
//         TotalFreshness += FreshnessFactor;
//     }
    
//     const float AverageFreshness = TotalFreshness / (EndIndex - MatchStartIdx);
    
//     // 新鲜度奖励：0-40分，随着输入的新鲜度增加而增加
//     return 40.0f * AverageFreshness;
// }

// float UComboScorer::CalculateRecentInputBonus(const FComboScoreContext& Context) const
// {
//     if (!Context.InputBufferPtr || Context.MatchedPatternStartIndex < 0)
//     {
//         return 0.0f;
//     }
    
//     const TArray<FCombatInputEntry>& InputBuffer = *Context.InputBufferPtr;
//     const int32 MatchStartIdx = Context.MatchedPatternStartIndex;
//     if (InputBuffer.Num() == 0 || MatchStartIdx < 0)
//     {
//         return 0.0f;
//     }
    
//     // 缓存常用值
//     //const TArray<FCombatInputEntry>& InputBuffer = Context.InputBuffer;
//     const int32 PatternLength = Context.MoveData->InputPattern.Num();
//     const int32 BufferSize = InputBuffer.Num();
//     const int32 EndIndex = FMath::Min(MatchStartIdx + PatternLength, BufferSize);
    
//     const float CurrentTime = Context.CurrentGameTime > 0.0f ? 
//                         Context.CurrentGameTime : 
//                         FPlatformTime::Seconds();
    
//     // 找出最近的输入时间戳
//     float MostRecentTimestamp = 0.0f;
//     for (int32 i = MatchStartIdx; i < EndIndex; i++)
//     {
//         MostRecentTimestamp = FMath::Max(MostRecentTimestamp, InputBuffer[i].Timestamp);
//     }
    
//     // 计算最近输入的年龄
//     const float MostRecentAge = CurrentTime - MostRecentTimestamp;
//     const float RecentThreshold = 0.3f; // 300ms阈值
    
//     // 最近输入奖励
//     if (MostRecentAge < RecentThreshold)
//     {
//         const float RecencyFactor = 1.0f - (MostRecentAge / RecentThreshold);
//         return 35.0f * RecencyFactor; // 最多35分的近期输入奖励
//     }
    
//     return 0.0f;
// }

float UComboScorer::CalculateFreshnessBonus(const FComboScoreContext& Context) const
{
    if (!Context.InputBufferPtr || Context.MatchedPatternStartIndex < 0)
    {
        return 0.0f;
    }
    
    const TArray<FCombatInputEntry>& InputBuffer = *Context.InputBufferPtr;
    const int32 MatchStartIdx = Context.MatchedPatternStartIndex;
    if (InputBuffer.Num() == 0 || MatchStartIdx < 0)
    {
        return 0.0f;
    }
    
    // 缓存常用值
    const int32 PatternLength = Context.MoveData->InputPattern.Num();
    const int32 BufferSize = InputBuffer.Num();
    const int32 EndIndex = FMath::Min(MatchStartIdx + PatternLength, BufferSize);
    
    if (EndIndex <= MatchStartIdx) return 0.0f;
    
    // 修改：统一使用Context.CurrentGameTime，不再回退到FPlatformTime::Seconds()
    const float CurrentTime = Context.CurrentGameTime;
    
    // 计算匹配的输入序列的平均新鲜度
    float TotalFreshness = 0.0f;
    const float InvLifespan = 1.0f / Context.DefaultInputLifespan; // 避免除法运算
    
    for (int32 i = MatchStartIdx; i < EndIndex; i++)
    {
        const float InputAge = CurrentTime - InputBuffer[i].Timestamp;
        const float FreshnessFactor = FMath::Clamp(1.0f - (InputAge * InvLifespan), 0.0f, 1.0f);
        TotalFreshness += FreshnessFactor;
    }
    
    const float AverageFreshness = TotalFreshness / (EndIndex - MatchStartIdx);
    
    // 新鲜度奖励：0-40分，随着输入的新鲜度增加而增加
    return 40.0f * AverageFreshness;
}

float UComboScorer::CalculateRecentInputBonus(const FComboScoreContext& Context) const
{
    if (!Context.InputBufferPtr || Context.MatchedPatternStartIndex < 0)
    {
        return 0.0f;
    }
    
    const TArray<FCombatInputEntry>& InputBuffer = *Context.InputBufferPtr;
    const int32 MatchStartIdx = Context.MatchedPatternStartIndex;
    if (InputBuffer.Num() == 0 || MatchStartIdx < 0)
    {
        return 0.0f;
    }
    
    // 缓存常用值
    const int32 PatternLength = Context.MoveData->InputPattern.Num();
    const int32 BufferSize = InputBuffer.Num();
    const int32 EndIndex = FMath::Min(MatchStartIdx + PatternLength, BufferSize);
    
    // 修改：统一使用Context.CurrentGameTime
    const float CurrentTime = Context.CurrentGameTime;
    
    // 找出最近的输入时间戳
    float MostRecentTimestamp = 0.0f;
    for (int32 i = MatchStartIdx; i < EndIndex; i++)
    {
        MostRecentTimestamp = FMath::Max(MostRecentTimestamp, InputBuffer[i].Timestamp);
    }
    
    // 计算最近输入的年龄
    const float MostRecentAge = CurrentTime - MostRecentTimestamp;
    const float RecentThreshold = 0.3f; // 300ms阈值
    
    // 最近输入奖励
    if (MostRecentAge < RecentThreshold)
    {
        const float RecencyFactor = 1.0f - (MostRecentAge / RecentThreshold);
        return 35.0f * RecencyFactor; // 最多35分的近期输入奖励
    }
    
    return 0.0f;
}