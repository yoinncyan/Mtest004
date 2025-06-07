# UE5《忍者龙剑传 II》连招系统 PRD — 扩展版（前置招式 & 测试示例）

> **本页专注于：** ① DataAsset 新字段 *PrereqComboSeq* ② 绝招 C 代码实现与模拟测试脚本

---

## 1. DataAsset 字段增补

| 字段               | 类型                                       | 说明                                       |
| ---------------- | ---------------------------------------- | ---------------------------------------- |
| `PrereqComboSeq` | `TArray<TSoftObjectPtr<UComboMoveData>>` | **前置招式序列**；若为空 ⇒ 无需前置条件。编辑器可拖入任意招式资产并排序。 |

> **校验规则（严格顺序）：**
>
> * 只有当玩家连续执行序列中的每一招且**期间未插入其他非序列成员**时，`PrereqSatisfied=true`。
> * 序列可以被“前置辅助动作”前置，如 `A0A1A2A3` 合法；但插入到内部则中断 (`A1A2B1A3` 非法)。

示例 `DA_C` 设置：`PrereqComboSeq={DA_A1,DA_A2,DA_A3,DA_B1,DA_B2}`。

---

## 2. CombatComponent 逻辑补丁（修正版）

> **重要差异**：允许序列开始前出现任意前置动作；只有在 *序列已开始* 后检测到“非预期动作”才会重置。

### 2.1 新增成员（不变）

```cpp
int32  SeqIndex = 0;                                          // 已匹配到的序列步数
TArray<TSoftObjectPtr<UComboMoveData>> TargetSequence;        // 运行期拷贝自资产
bool    bPrereqReady = false;                                 // 完成标志
TObjectPtr<UComboMoveData> CachedSpecialMove = nullptr;       // 绝招 C
```

### 2.2 缓存序列（不变）

```cpp
CachedSpecialMove = MoveTable.FindRef("C");
if (CachedSpecialMove) TargetSequence = CachedSpecialMove->PrereqComboSeq;
```

### 2.3 序列推进核心算法

```cpp
// 仅当定义了序列时才进入比对
if (TargetSequence.Num() > 0)
{
    const bool bMatch =                               /* 当前是否匹配期待动作 */
        TargetSequence.IsValidIndex(SeqIndex) &&
        CurrentMove == TargetSequence[SeqIndex];

    if (bMatch)
    {
        ++SeqIndex;                                   // 推进步数
        LOG_SEQ(%s 命中, SeqIndex, TargetSequence.Num());

        if (SeqIndex == TargetSequence.Num())
        {
            bPrereqReady = true;                      // 序列完成
            UE_LOG(LogTemp, Warning, TEXT("[Seq] FULL ready!"));
        }
    }
    else if (SeqIndex > 0 && !CurrentMove->bIgnoreForSequence)
    {
        // 只有序列已经开始且当前动作不匹配时才重置
        UE_LOG(LogTemp, Log, TEXT("[Seq] Break @%s  Reset!"), *CurrentMove->MoveID.ToString());
        SeqIndex = 0;
        bPrereqReady = false;
    }
    // 若 SeqIndex == 0 且未匹配 → 忽略，允许前置杂招
}
```

### 2.4 触发绝招（不变）

```cpp
if (Key==Light && bPrereqReady && CachedSpecialMove)
{
    ExecuteSimulatedMove(CachedSpecialMove);
    SeqIndex     = 0;
    bPrereqReady = false;
    return;
}
```

---

## 3. 测试用例修订

| 用例     | 输入                    | 预期               |
| ------ | --------------------- | ---------------- |
| POS‑03 | **B3 A1 A2 A3 Light** | 前缀非序列动作允许 → C 触发 |
| NEG‑05 | **A1 A2 B1 A3 Light** | 序列中插入非成员 → C 不触发 |

---
