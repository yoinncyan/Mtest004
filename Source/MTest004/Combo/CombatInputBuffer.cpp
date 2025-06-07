// CombatInputBuffer.cpp
#include "CombatInputBuffer.h"
#include "ComboScorer.h" // Include the new Scorer header
#include "ComboMoveData.h" // Ensure UComboMoveData type is available

// Constructor Definition
UCombatInputBuffer::UCombatInputBuffer()
    : bInChainWindow(false)
    , bUseChainBuffer(false)
    , CurrentFlexMask(0)
    , LastInputTime(0.0f)
    , GlobalBufferHead(0)
    , GlobalBufferCount(0)
    , ChainBufferHead(0)
    , ChainBufferCount(0)
{
    // 预分配固定大小的存储空间
    GlobalBufferStorage.SetNumZeroed(GLOBAL_BUFFER_SIZE);
    ChainBufferStorage.SetNumZeroed(CHAIN_BUFFER_SIZE);
    PrimaryComponentTick.bCanEverTick = false; // Typically, input buffers don't need to tick themselves
}

// 在BeginPlay中设置定时清理
void UCombatInputBuffer::BeginPlay()
{
    Super::BeginPlay();
    
    // 每0.2秒清理一次过期输入
    GetWorld()->GetTimerManager().SetTimer(
        CleanupTimerHandle,
        this,
        &UCombatInputBuffer::CleanExpiredInputs,
        0.2f,
        true
    );
}

//使用环形缓冲区
void UCombatInputBuffer::ProcessInput(ECombatInputKey Key, const FVector2D& StickInput)
{
    const float CurrentTime = FPlatformTime::Seconds();
    
    FCombatInputEntry NewEntry;
    NewEntry.Key = Key;
    NewEntry.Direction = StickInput;
    NewEntry.Timestamp = CurrentTime;
    
    // 全局缓冲管理 - 使用环形缓冲区
    if(GlobalBufferCount < GLOBAL_BUFFER_SIZE)
    {
        // 缓冲区未满，直接添加
        int32 PhysicalIdx = GetPhysicalIndex(GlobalBufferCount, GlobalBufferHead, GLOBAL_BUFFER_SIZE);
        GlobalBufferStorage[PhysicalIdx] = NewEntry;
        GlobalBufferCount++;
    }
    else
    {
        // 缓冲区已满，覆盖最旧的输入并移动头指针
        GlobalBufferStorage[GlobalBufferHead] = NewEntry;
        GlobalBufferHead = (GlobalBufferHead + 1) % GLOBAL_BUFFER_SIZE;
    }
    
    // 链式缓冲特殊处理
    if(bInChainWindow)
    {
        if(ChainBufferCount < CHAIN_BUFFER_SIZE)
        {
            // 链缓冲未满，直接添加
            int32 PhysicalIdx = GetPhysicalIndex(ChainBufferCount, ChainBufferHead, CHAIN_BUFFER_SIZE);
            ChainBufferStorage[PhysicalIdx] = NewEntry;
            ChainBufferCount++;
        }
        else
        {
            // 链缓冲已满，覆盖最旧的输入并移动头指针
            ChainBufferStorage[ChainBufferHead] = NewEntry;
            ChainBufferHead = (ChainBufferHead + 1) % CHAIN_BUFFER_SIZE;
        }
    }
    
    LastInputTime = CurrentTime;
}

// void UCombatInputBuffer::ProcessInput(ECombatInputKey Key, const FVector2D& StickInput)
// {
//     //const float CurrentTime = GetWorld()->GetTimeSeconds();
//     // 使用高精度时间而不是游戏时间
//     const float CurrentTime = FPlatformTime::Seconds();
    
//     FCombatInputEntry NewEntry;
//     NewEntry.Key = Key;
//     NewEntry.Direction = StickInput;
//     NewEntry.Timestamp = CurrentTime;
    
//     // 全局缓冲管理
//     if(GlobalBuffer.Num() >= GLOBAL_BUFFER_SIZE) GlobalBuffer.RemoveAt(0);
//     GlobalBuffer.Add(NewEntry);
    
//     // 链式缓冲特殊处理
//     if(bInChainWindow && ChainBuffer.Num() < CHAIN_BUFFER_SIZE)
//     {
//         ChainBuffer.Add(NewEntry);
//     }
    
//     LastInputTime = CurrentTime;
// }

// Modified MatchPattern to support returning score or detailed match info
// For simplicity, we'll keep MatchPattern for boolean checks, 
// and add a new function or modify CombatComponent to handle scoring.

bool UCombatInputBuffer::MatchPattern(const TArray<ECombatInputKey>& Pattern, int32 FlexBits) const
{
    const auto& BufferToUse = bUseChainBuffer ? GetFilteredChainBuffer() : GetFilteredBuffer();
    return FindPatternMatch(Pattern, BufferToUse, FlexBits).bFound;
}

// New function to find the best match and return its score context details
// Returns the start index of the best match in the buffer, or -1 if no match.
int32 UCombatInputBuffer::FindBestMatchStartIndex(const UComboMoveData* MoveData, int32 FlexBits, int32* OutFlexBitsUsed) const
{
    if (!MoveData || MoveData->InputPattern.Num() == 0) return -1;
    
    const auto& BufferToUse = bUseChainBuffer ? GetFilteredChainBuffer() : GetFilteredBuffer();
    FPatternMatchResult Result = FindPatternMatch(MoveData->InputPattern, BufferToUse, FlexBits);
    
    // 设置输出参数
    if (OutFlexBitsUsed)
    {
        *OutFlexBitsUsed = Result.FlexBitsUsed;
    }
    
    return Result.bFound ? Result.StartIndex : -1;
}

void UCombatInputBuffer::AddInput(ECombatInputKey Key)
{
    ProcessInput(Key, FVector2D::ZeroVector);
}

void UCombatInputBuffer::ClearBuffer()
{
    //GlobalBuffer.Empty();
    //ChainBuffer.Empty();
    GlobalBufferHead = 0;
    GlobalBufferCount = 0;
    ChainBufferHead = 0;
    ChainBufferCount = 0;
}

void UCombatInputBuffer::RemoveOldestInput()
{
    // if (GlobalBuffer.Num() > 0)
    // {
    //     GlobalBuffer.RemoveAt(0);
    // }
    if (GlobalBufferCount > 0)
    {
        // 仅移动头指针，无需移动数据
        GlobalBufferHead = (GlobalBufferHead + 1) % GLOBAL_BUFFER_SIZE;
        GlobalBufferCount--;
    }
}

TArray<ECombatInputKey> UCombatInputBuffer::GetCurrentInputs() const
{
    TArray<ECombatInputKey> CurrentInputs;
    for (const FCombatInputEntry& Entry : GlobalBuffer)
    {
        CurrentInputs.Add(Entry.Key);
    }
    return CurrentInputs;
}

// 获取过滤后的有效输入
TArray<FCombatInputEntry> UCombatInputBuffer::GetFilteredBuffer(float MaxAge) const
{
    TArray<FCombatInputEntry> FilteredEntries;
    float CurrentTime = FPlatformTime::Seconds();
    float TimeThreshold = (MaxAge > 0.0f) ? MaxAge : DefaultInputLifespan;
    
    // 遍历环形缓冲区，只添加有效期内的输入
    for(int32 i = 0; i < GlobalBufferCount; ++i)
    {
        int32 PhysicalIdx = GetPhysicalIndex(i, GlobalBufferHead, GLOBAL_BUFFER_SIZE);
        const FCombatInputEntry& Entry = GlobalBufferStorage[PhysicalIdx];
        
        if (CurrentTime - Entry.Timestamp <= TimeThreshold)
        {
            FilteredEntries.Add(Entry);
        }
    }
    
    return FilteredEntries;
}
// TArray<FCombatInputEntry> UCombatInputBuffer::GetFilteredBuffer(float MaxAge) const
// {
//     TArray<FCombatInputEntry> FilteredEntries;
//     float CurrentTime = FPlatformTime::Seconds();
//     float TimeThreshold = (MaxAge > 0.0f) ? MaxAge : DefaultInputLifespan;
    
//     for (const FCombatInputEntry& Entry : GlobalBuffer)
//     {
//         if (CurrentTime - Entry.Timestamp <= TimeThreshold)
//         {
//             FilteredEntries.Add(Entry);
//         }
//     }
    
//     return FilteredEntries;
// }

// 添加函数实现
TArray<FCombatInputEntry> UCombatInputBuffer::GetFilteredChainBuffer(float MaxAge) const
{
    TArray<FCombatInputEntry> FilteredEntries;
    float CurrentTime = FPlatformTime::Seconds();
    float TimeThreshold = (MaxAge > 0.0f) ? MaxAge : DefaultInputLifespan;
    
    for (const FCombatInputEntry& Entry : ChainBuffer)
    {
        if (CurrentTime - Entry.Timestamp <= TimeThreshold)
        {
            FilteredEntries.Add(Entry);
        }
    }
    
    return FilteredEntries;
}

// 清理过期输入
void UCombatInputBuffer::CleanExpiredInputs()
{
    float CurrentTime = FPlatformTime::Seconds();
    int32 FirstValidIndex = GlobalBuffer.Num(); // 默认全部删除
    
    // 找到第一个未过期的输入的索引
    for (int32 i = 0; i < GlobalBuffer.Num(); i++)
    {
        if (CurrentTime - GlobalBuffer[i].Timestamp <= DefaultInputLifespan)
        {
            FirstValidIndex = i;
            break;
        }
    }
    
    // 删除所有过期输入
    if (FirstValidIndex > 0)
    {
        GlobalBuffer.RemoveAt(0, FirstValidIndex);
        UE_LOG(LogTemp, Verbose, TEXT("Auto-cleaned %d expired inputs"), FirstValidIndex);
    }
}

// 添加功能，检查PreserveBuffer标志时选择性保留最近的N个输入
void UCombatInputBuffer::PreserveRecentInputs(int32 CountToPreserve = 1)
{
    // auto FilteredInputs = GetFilteredBuffer();
    // int32 InputCount = FilteredInputs.Num();
    
    // if (InputCount <= CountToPreserve) {
    //     return; // 不需要清理
    // }
    
    // // 只保留最近的N个输入
    // GlobalBuffer.Empty();
    // for (int32 i = InputCount - CountToPreserve; i < InputCount; i++) {
    //     GlobalBuffer.Add(FilteredInputs[i]);
    // }
    if (CountToPreserve <= 0 || GlobalBufferCount <= CountToPreserve)
    {
        return; // 不需要操作
    }
    
    // 计算要保留的起始逻辑索引
    int32 StartPreserveIdx = GlobalBufferCount - CountToPreserve;
    
    // 重新设置头指针和计数
    GlobalBufferHead = GetPhysicalIndex(StartPreserveIdx, GlobalBufferHead, GLOBAL_BUFFER_SIZE);
    GlobalBufferCount = CountToPreserve;
}

// 优化版也使用统一逻辑，但直接在原始缓冲区上操作
int32 UCombatInputBuffer::FindBestMatchStartIndexOptimized(const UComboMoveData* MoveData, 
                                                        int32 FlexBits, 
                                                        int32* OutFlexBitsUsed,
                                                        float MaxAge) const
{
    if (!MoveData || MoveData->InputPattern.Num() == 0) return -1;
    
    // 获取适当的缓冲区引用
    const TArray<FCombatInputEntry>& RawBuffer = bUseChainBuffer ? ChainBuffer : GlobalBuffer;
    
    // 使用统一匹配函数，传递MaxAge参数以启用时间过滤
    FPatternMatchResult Result = FindPatternMatch(MoveData->InputPattern, RawBuffer, FlexBits, 
                                                  MaxAge > 0.0f ? MaxAge : DefaultInputLifespan);
    
    // 设置输出参数
    if (OutFlexBitsUsed && Result.bFound)
    {
        *OutFlexBitsUsed = Result.FlexBitsUsed;
    }
    
    return Result.StartIndex; // 统一结果已经有-1的默认值
}

// 重写获取缓冲区方法，返回线性视图
TArray<FCombatInputEntry> UCombatInputBuffer::GetGlobalBuffer() const
{
    TArray<FCombatInputEntry> LinearBuffer;
    LinearBuffer.Reserve(GlobalBufferCount);
    
    // 将环形缓冲区转换为线性数组
    for(int32 i = 0; i < GlobalBufferCount; ++i)
    {
        int32 PhysicalIdx = GetPhysicalIndex(i, GlobalBufferHead, GLOBAL_BUFFER_SIZE);
        LinearBuffer.Add(GlobalBufferStorage[PhysicalIdx]);
    }
    
    return LinearBuffer;
}

TArray<FCombatInputEntry> UCombatInputBuffer::GetChainBuffer() const
{
    TArray<FCombatInputEntry> LinearBuffer;
    LinearBuffer.Reserve(ChainBufferCount);
    
    // 将环形缓冲区转换为线性数组
    for(int32 i = 0; i < ChainBufferCount; ++i)
    {
        int32 PhysicalIdx = GetPhysicalIndex(i, ChainBufferHead, CHAIN_BUFFER_SIZE);
        LinearBuffer.Add(ChainBufferStorage[PhysicalIdx]);
    }
    
    return LinearBuffer;
}

// 优化版本：视图式过滤，避免拷贝缓冲区
// int32 UCombatInputBuffer::FindBestMatchStartIndexOptimized(const UComboMoveData* MoveData, 
//                                                         int32 FlexBits, 
//                                                         int32* OutFlexBitsUsed,
//                                                         float MaxAge) const
// {
//     if (!MoveData || MoveData->InputPattern.Num() == 0) return -1;

//     // 获取原始缓冲区的引用而非拷贝
//     const TArray<FCombatInputEntry>& RawBuffer = bUseChainBuffer ? ChainBuffer : GlobalBuffer;
//     const TArray<ECombatInputKey>& Pattern = MoveData->InputPattern;
    
//     // 如果原始缓冲区太小，直接返回
//     if (RawBuffer.Num() < Pattern.Num()) return -1;
    
//     // 使用的时间阈值
//     float CurrentTime = FPlatformTime::Seconds();
//     float TimeThreshold = (MaxAge > 0.0f) ? MaxAge : DefaultInputLifespan;

//     int32 BestMatchStartIndex = -1;
//     int32 BestFlexBitsUsed = 0;
    
//     // 在原始缓冲区上直接查找匹配，但在访问每个元素时检查时间戳
//     for (int32 StartIdx = 0; StartIdx <= RawBuffer.Num() - Pattern.Num(); ++StartIdx)
//     {
//         bool bCurrentMatch = true;
//         int32 CurrentFlexBitsUsed = 0;
//         bool bHasExpiredInput = false;
        
//         for (int32 i = 0; i < Pattern.Num(); ++i)
//         {
//             const FCombatInputEntry& Entry = RawBuffer[StartIdx + i];
            
//             // 检查这个输入是否过期
//             if (!IsInputValid(Entry, CurrentTime, TimeThreshold))
//             {
//                 bHasExpiredInput = true;
//                 break;
//             }
            
//             const ECombatInputKey TargetKey = Pattern[i];
            
//             if (Entry.Key != TargetKey)
//             {
//                 // 检查这个位置是否允许灵活输入
//                 if (FlexBits & (1 << i))
//                 {
//                     CurrentFlexBitsUsed |= (1 << i); // 标记这个弹性位已使用
//                 }
//                 else
//                 {
//                     bCurrentMatch = false;
//                     break;
//                 }
//             }
//         }
        
//         // 如果当前窗口包含过期输入，跳到下一个窗口
//         if (bHasExpiredInput) continue;
        
//         // 找到有效匹配
//         if (bCurrentMatch)
//         {
//             BestMatchStartIndex = StartIdx;
//             BestFlexBitsUsed = CurrentFlexBitsUsed;
//             break;
//         }
//     }
    
//     // 设置输出参数
//     if (OutFlexBitsUsed)
//     {
//         *OutFlexBitsUsed = BestFlexBitsUsed;
//     }
    
//     return BestMatchStartIndex;
// }

// 实现统一的模式匹配函数:

// UCombatInputBuffer::FPatternMatchResult UCombatInputBuffer::FindPatternMatch(const TArray<ECombatInputKey>& Pattern, const TArray<FCombatInputEntry>& Buffer,int32 FlexBits,float MaxAge) const
// {
//     FPatternMatchResult Result;
//     float CurrentTime = FPlatformTime::Seconds();
//     float TimeThreshold = (MaxAge > 0.0f) ? MaxAge : DefaultInputLifespan;
    
//     // 没有模式或缓冲区太小，直接返回未匹配
//     if (Pattern.Num() == 0 || Buffer.Num() < Pattern.Num()) {
//         return Result;
//     }
    
//     // 滑动窗口匹配
//     for (int32 StartIdx = 0; StartIdx <= Buffer.Num() - Pattern.Num(); ++StartIdx)
//     {
//         bool bCurrentMatch = true;
//         int32 CurrentFlexBitsUsed = 0;
//         bool bHasExpiredInput = false;
        
//         for (int32 i = 0; i < Pattern.Num(); ++i)
//         {
//             const FCombatInputEntry& Entry = Buffer[StartIdx + i];
            
//             // 如果指定了时间阈值，检查输入是否过期
//             if (MaxAge >= 0.0f && (CurrentTime - Entry.Timestamp > TimeThreshold))
//             {
//                 bHasExpiredInput = true;
//                 break;
//             }
            
//             const ECombatInputKey TargetKey = Pattern[i];
            
//             if (Entry.Key != TargetKey)
//             {
//                 // 检查这个位置是否允许弹性输入
//                 if (FlexBits & (1 << i))
//                 {
//                     CurrentFlexBitsUsed |= (1 << i); // 标记这个弹性位已使用
//                 }
//                 else
//                 {
//                     bCurrentMatch = false;
//                     break;
//                 }
//             }
//         }
        
//         // 跳过包含过期输入的窗口
//         if (bHasExpiredInput) continue;
        
//         // 找到匹配
//         if (bCurrentMatch)
//         {
//             Result.bFound = true;
//             Result.StartIndex = StartIdx;
//             Result.FlexBitsUsed = CurrentFlexBitsUsed;
//             return Result; // 立即返回第一个匹配
//         }
//     }
    
//     return Result; // 未找到匹配
// }

UCombatInputBuffer::FPatternMatchResult UCombatInputBuffer::FindPatternMatch(
    const TArray<ECombatInputKey>& Pattern, 
    const TArray<FCombatInputEntry>& Buffer,
    int32 FlexBits,
    float MaxAge) const
{
    FPatternMatchResult Result;
    float CurrentTime = FPlatformTime::Seconds();
    float TimeThreshold = (MaxAge > 0.0f) ? MaxAge : DefaultInputLifespan;
    
    // 没有模式或缓冲区太小，直接返回未匹配
    if (Pattern.Num() == 0 || Buffer.Num() < Pattern.Num()) {
        return Result;
    }
    
    // 滑动窗口匹配
    for (int32 StartIdx = 0; StartIdx <= Buffer.Num() - Pattern.Num(); ++StartIdx)
    {
        bool bCurrentMatch = true;
        int32 CurrentFlexBitsUsed = 0;
        bool bHasExpiredInput = false;
        
        for (int32 i = 0; i < Pattern.Num(); ++i)
        {
            const FCombatInputEntry& Entry = Buffer[StartIdx + i];
            
            // 如果指定了时间阈值，检查输入是否过期
            if (MaxAge >= 0.0f && (CurrentTime - Entry.Timestamp > TimeThreshold))
            {
                bHasExpiredInput = true;
                break;
            }
            
            const ECombatInputKey TargetKey = Pattern[i];
            
            if (Entry.Key != TargetKey)
            {
                // 检查这个位置是否允许弹性输入
                if (FlexBits & (1 << i))
                {
                    CurrentFlexBitsUsed |= (1 << i); // 标记这个弹性位已使用
                }
                else
                {
                    bCurrentMatch = false;
                    break;
                }
            }
        }
        
        // 跳过包含过期输入的窗口
        if (bHasExpiredInput) continue;
        
        // 找到匹配
        if (bCurrentMatch)
        {
            Result.bFound = true;
            Result.StartIndex = StartIdx;
            Result.FlexBitsUsed = CurrentFlexBitsUsed;
            return Result; // 立即返回第一个匹配
        }
    }
    
    return Result; // 未找到匹配
}