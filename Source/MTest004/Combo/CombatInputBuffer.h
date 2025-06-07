// CombatInputBuffer.h
#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CombatInputBuffer.generated.h"

UENUM(BlueprintType)
enum class ECombatInputKey : uint8
{
    CIK_Light,
    CIK_Heavy,
    CIK_Jump,
    CIK_Block,
    CIK_MAX
};

USTRUCT(BlueprintType)
struct FCombatInputEntry
{
    GENERATED_BODY()
    
    UPROPERTY(VisibleAnywhere)
    ECombatInputKey Key;
    
    UPROPERTY(VisibleAnywhere)
    FVector2D Direction;
    
    UPROPERTY(VisibleAnywhere)
    float Timestamp;

    // 添加默认构造函数
    FCombatInputEntry()
        : Key(ECombatInputKey::CIK_Light), Direction(FVector2D::ZeroVector), Timestamp(0.0f) {}
    
    // 保留现有的带参数构造函数
    FCombatInputEntry(ECombatInputKey InKey) 
        : Key(InKey), Direction(FVector2D::ZeroVector), Timestamp(FPlatformTime::Seconds()) {}
};

UCLASS(ClassGroup=(Combat), meta=(BlueprintSpawnableComponent))
class MTEST004_API UCombatInputBuffer : public UActorComponent
{
    GENERATED_BODY()
public:
    UCombatInputBuffer(); // Constructor declaration

    static constexpr int32 GLOBAL_BUFFER_SIZE = 6; // 6逻辑帧存储
    static constexpr int32 CHAIN_BUFFER_SIZE = 2;

    virtual void BeginPlay() override;

    void ProcessInput(ECombatInputKey Key, const FVector2D& StickInput);
    bool MatchPattern(const TArray<ECombatInputKey>& Pattern, int32 FlexBits) const;
    int32 FindBestMatchStartIndex(const class UComboMoveData* MoveData, int32 FlexBits, int32* OutFlexBitsUsed = nullptr) const;    // Getter for accessing the buffer from external components
    
    // UFUNCTION(BlueprintPure, Category = "Input")
    // const TArray<FCombatInputEntry>& GetGlobalBuffer() const { return GlobalBuffer; }

    // UFUNCTION(BlueprintPure, Category = "Input")
    // const TArray<FCombatInputEntry>& GetChainBuffer() const { return ChainBuffer; }

    // Additional utility functions
    UFUNCTION(BlueprintCallable, Category = "Input")
    void AddInput(ECombatInputKey Key);

    UFUNCTION(BlueprintCallable, Category = "Input")
    void ClearBuffer();

    UFUNCTION(BlueprintCallable, Category = "Input")
    void RemoveOldestInput();

    UFUNCTION(BlueprintPure, Category = "Input")
    TArray<ECombatInputKey> GetCurrentInputs() const;

protected:
    TArray<FCombatInputEntry> GlobalBuffer;
    TArray<FCombatInputEntry> ChainBuffer;

    bool bInChainWindow = false;    // Added
    bool bUseChainBuffer = false;   // Added
    
    uint8 CurrentFlexMask = 0;
    float LastInputTime = 0.0f;

public:
    // 添加获取过滤后缓冲区的方法
    UFUNCTION(BlueprintPure, Category = "Input")
    TArray<FCombatInputEntry> GetFilteredBuffer(float MaxAge = -1.0f) const;

    UFUNCTION(BlueprintPure, Category = "Input")
    TArray<FCombatInputEntry> GetFilteredChainBuffer(float MaxAge = -1.0f) const;
    
    // 清理过期输入
    UFUNCTION(BlueprintCallable, Category = "Input")
    void CleanExpiredInputs();

    void PreserveRecentInputs(int32 CountToPreserve);

        // 新增：不创建副本的模式匹配函数
    int32 FindBestMatchStartIndexOptimized(const UComboMoveData* MoveData, int32 FlexBits, 
                                         int32* OutFlexBitsUsed = nullptr, float MaxAge = -1.0f) const;


    // 模式匹配结果结构体
    struct FPatternMatchResult
    {
        bool bFound = false;         // 是否找到匹配
        int32 StartIndex = -1;       // 匹配的起始索引
        int32 FlexBitsUsed = 0;      // 使用的弹性位
    };

        // 改为使用固定大小数组 - 预分配但不会触发重新分配
    TArray<FCombatInputEntry> GlobalBufferStorage;
    TArray<FCombatInputEntry> ChainBufferStorage;
    
    // 环形缓冲区索引
    int32 GlobalBufferHead = 0;    // 指向最旧的输入
    int32 GlobalBufferCount = 0;   // 当前缓冲区中的元素数量
    
    int32 ChainBufferHead = 0;
    int32 ChainBufferCount = 0;
    
    // 根据逻辑索引获取物理存储索引
    FORCEINLINE int32 GetPhysicalIndex(int32 LogicalIndex, int32 Head, int32 Capacity) const
    {
        return (Head + LogicalIndex) % Capacity;
    }

        // 重写获取全局缓冲区的方法，返回线性视图而非环形缓冲内部表示
    UFUNCTION(BlueprintPure, Category = "Input")
    TArray<FCombatInputEntry> GetGlobalBuffer() const;
    
    // 重写获取链缓冲区的方法
    UFUNCTION(BlueprintPure, Category = "Input")
    TArray<FCombatInputEntry> GetChainBuffer() const;
    
    // 添加方法获取缓冲区中特定索引的元素
    FORCEINLINE FCombatInputEntry& GetGlobalBufferEntry(int32 LogicalIndex)
    {
        check(LogicalIndex >= 0 && LogicalIndex < GlobalBufferCount);
        int32 PhysicalIdx = GetPhysicalIndex(LogicalIndex, GlobalBufferHead, GLOBAL_BUFFER_SIZE);
        return GlobalBufferStorage[PhysicalIdx];
    }
    
    FORCEINLINE const FCombatInputEntry& GetGlobalBufferEntry(int32 LogicalIndex) const
    {
        check(LogicalIndex >= 0 && LogicalIndex < GlobalBufferCount);
        int32 PhysicalIdx = GetPhysicalIndex(LogicalIndex, GlobalBufferHead, GLOBAL_BUFFER_SIZE);
        return GlobalBufferStorage[PhysicalIdx];
    }
    
    // 输入有效期配置
    UPROPERTY(EditDefaultsOnly, Category = "Input", meta = (ClampMin = "0.1", ClampMax = "2.0"))
    float DefaultInputLifespan = 0.8f;  // 默认输入有效期，单位秒
    
    // 定时清理用的定时器句柄
    FTimerHandle CleanupTimerHandle;

    // 判断输入是否在有效时间范围内
    FORCEINLINE bool IsInputValid(const FCombatInputEntry& Entry, float CurrentTime, float MaxAge) const
    {
        return (CurrentTime - Entry.Timestamp <= MaxAge);
    }
    
    // 统一的匹配函数，同时处理普通匹配和带时间戳匹配
    FPatternMatchResult FindPatternMatch(
        const TArray<ECombatInputKey>& Pattern, 
        const TArray<FCombatInputEntry>& Buffer,
        int32 FlexBits,
        float MaxAge = -1.0f) const;
};
