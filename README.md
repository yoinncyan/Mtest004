# Mtest004

# UE5《忍者龙剑传 II》连招系统 PRD v0.33（中文整合版）

> **代号：**Blade Flow／NG2 Mode **动画帧率：**60 fps **逻辑步频：**120 Hz **文档负责人：**战斗设计组

---

## 0 版本记录

| 版本       | 日期         | 关键更新                                     |
| -------- | ---------- | ------------------------------------------ |
| 0.31     | 2025‑06‑02 | 统一章节编号，补充异常处理 & 状态转换                  |
| 0.32     | 2025‑06‑02 | 中文化；新增相机动态总览 & 敌人类型扩展预留                |
| **0.33** | 2025‑06‑05 | **整合“扩展版 PRD”需求，新增前置招式序列逻辑与测试用例** |

---

## 1 目标

| 编号  | 目标              | 指标           |
| --- | --------------- | ------------ |
| G‑1 | 缓冲误触发率 ≤ 1 %    | 1 000 条压力用例  |
| G‑2 | 角色控制延迟 ≤ 55 ms  | Startup+输入延迟 |
| G‑3 | 标志性招式成功率 ≥ 97 % | 双燕／落凤饭／风步    |

---

## 2 输入与缓冲

两级缓冲：**全局**2键（≤6 逻辑帧）+ **链式**2键；阶段切换强制清空。
### 2.2 最近匹配 Scorer

`Score = Base + OrderBonus – Δt Penalty + FlexBonus`
可通过 *DataAsset* 指派自定义 `UComboScorer` 子类覆盖（例如飞燕提高 Y 权重）。

---

## 3 角色状态机

### 3.1 主状态

```
Idle
 ├─ Ground
 │   ├─ Run
 │   └─ Defensive
 ├─ Air
 │   └─ FS_Rebound
 ├─ Wall
 └─ WaterRun (预留)
```

### 3.2 状态转换表

| 起始 → 目标            | 触发条件                                    | 优先级 | 备注                    |
| ------------------ | --------------------------------------- | --- | --------------------- |
| Ground → Air       | `Jump` 输入 **或** `Launcher` Tag OnHit    | 3   | 设置 `bIsAirborne`      |
| Air → Ground       | `bOnFloor==true`                        | –   | 自动                    |
| Ground ↔ Run       | `MoveAxis ≥ 0.7` 持续 3f／<0.3 持续 5f       | 4   |                       |
| Any → Wall         | `bWallDetect==true` & `Jump`            | 5   | 增设 `WallTimer=900 ms` |
| Wall → Air         | `WallKick` 动作播放完 **或** Timer 到          | –   |                       |
| Any → Defensive    | `Block` 在 `ParryWindow` 或 `WindRun` 输入  | 6   | 无敌帧 8f                |
| Defensive → Ground | 防御动作 Recovery 完成                        | –   |                       |
| Air ↔ FS\_Rebound  | Flying Swallow OnHit 设置 ↔ 18f 后自动退回 Air | 2   |                       |

> **优先级**：高数值可打断低数值。

---

## 4 DataAsset字段
### 4.1 `UComboMoveData` 主要字段（★ = 本版新增/整合）

| 字段                          | 类型                                       | 说明                                                                |
| --------------------------- | ---------------------------------------- | ----------------------------------------------------------------- |
| MoveID                      | `FName`                                  | 唯一标识                                                              |
| DisplayName                 | `FText`                                  | UI / Debug                                                        |
| InputPattern                | `TArray<EInputKey>`                      | 逻辑键序列 (≤4)                                                      |
| PatternFlex                 | `uint8`                                  | 位图容错                                                              |
| DirectionMask               | `uint8`                                  | ↑↓→← 或 Neutral                                                   |
| ValidState                  | `ECombatState`                           | Ground / Air / Run / Wall                                         |
| Startup / Active / Recovery | `uint8×3`                                | 帧数据                                                               |
| ComboWindowStart / End      | `uint8`                                  | Recovery 子区段                                                     |
| NextMoves                   | `TArray<FName>`                          | 普通连招衔接                                                             |
| CancelMask                  | `uint8`                                  | 层级位                                                               |
| CancelCondition             | `bitflags`                               | OnHit / OnBlock / OnWhiff / OnKill                                |
| ChainRule                   | `FChainRule`                             | 见 4.2                                                              |
| PreserveBuffer              | `bool`                                   | 阶段切换保留 1 键                                                      |
| TargetPolicy                | `ETargetPolicy`                          | Nearest / Locked / Prev                                           |
| ResourceCost                | `uint8`                                  | 气 / 查克拉                                                            |
| Damage                      | `float`                                  | 基础伤害                                                              |
| DamageScaling               | `CurveFloat*`                            | 深连段递增 / 递减                                                      |
| Tags                        | `FGameplayTagContainer`                  | `Signature`,`Launcher` 等                                           |
| MinWeaponLevel              | `uint8`                                  | Lv gating                                                         |
| FXTagArray                  | `TArray<FName>`                          | 绑定 VFX/SFX                                                        |
| ★ `PrereqComboSeq`          | `TArray<TSoftObjectPtr<UComboMoveData>>` | **前置招式序列**；若为空 ⇒ 无需前置条件。编辑器可拖入任意招式资产并排序。                     |
| ★ `bIgnoreForSequence`      | `bool`                                   | `true` = 执行此招式时不重置 `PrereqComboSeq` 的进度。                  |

### 4.2 `FChainRule` 结构

| 字段               | 说明                                |
| ---------------- | --------------------------------- |
| MaxLoop          | 1‑3（飞燕 Lv1=1, Lv2=2, Lv3=3）       |
| TriggerCondition | Enum (OnHit / OnKill / Always)    |
| ChainWindow      | logic frames，输入有效窗口               |
| AutoAction       | 可选：Trigger 时自动插入 Jump / Rebound 等 |

> **运行时**：当 Move 触发且满足 TriggerCondition 时，`CombatSubsystem` 创建 LocalChainBuffer 并计时 `ChainWindow`。期间若匹配同 Move 的 `InputPattern` → ChainCount++ → 继续；否则窗口结束自动落地。
已包含 `ExecCondition · ChargeLevels · ParryWindow · LifeSteal` 等。

---

## 5. 前置招式序列 (Prerequisite Combo Sequence) 处理逻辑

本节描述了在 `CombatComponent` 中处理 `PrereqComboSeq` 的核心逻辑，允许某些特殊招式（例如“绝招 C”）需要在特定招式序列成功执行后才能触发。

> **重要差异**：允许序列开始前出现任意前置动作；只有在 *序列已开始* 后检测到“非预期动作”才会重置。

### 5.1 新增成员变量 (CombatComponent.h)

```cpp
// ... 其他成员 ...
int32  SeqIndex = 0;                                          // 已匹配到的前置序列步数
TArray<TSoftObjectPtr<UComboMoveData>> TargetSequence;        // 运行期从 CachedSpecialMove->PrereqComboSeq 拷贝
bool   bPrereqReady = false;                                  // 前置序列完成标志
TObjectPtr<UComboMoveData> CachedSpecialMove = nullptr;       // 缓存需要前置序列的特殊招式 (例如 "绝招 C")
// ... 其他成员 ...
```

### 5.2 缓存特殊招式及其前置序列 (例如在 BeginPlay 或初始化时)

```cpp
// 假设 MoveTable 已经加载了所有招式 DataAsset
CachedSpecialMove = MoveTable.FindRef("C"); // "C" 是特殊招式的 MoveID
if (CachedSpecialMove && CachedSpecialMove->PrereqComboSeq.Num() > 0)
{
    TargetSequence = CachedSpecialMove->PrereqComboSeq;
    UE_LOG(LogTemp, Log, TEXT("[Seq] Special move %s cached with %d prerequisite moves."), *CachedSpecialMove->MoveID.ToString(), TargetSequence.Num());
}
```

### 5.3 序列推进核心算法 (在每次成功执行一个招式后调用，例如在 `ExecuteSimulatedMove` 或实际的招式执行函数末尾)

```cpp
// 参数 CurrentMove: 当前刚刚成功执行的招式 (const UComboMoveData*)
// 仅当定义了目标序列 (TargetSequence) 且 CachedSpecialMove 有效时才进入比对
if (CachedSpecialMove && TargetSequence.Num() > 0)
{
    const bool bMatch = TargetSequence.IsValidIndex(SeqIndex) &&
                        CurrentMove == TargetSequence[SeqIndex].Get(); // 注意使用 .Get() 来获取 TSoftObjectPtr 的实际对象

    if (bMatch)
    {
        ++SeqIndex; // 推进步数
        // LOG_SEQ(%s 命中, SeqIndex, TargetSequence.Num()); // 假设 LOG_SEQ 是一个宏
        UE_LOG(LogTemp, Log, TEXT("[Seq] Move %s matched. Progress: %d/%d"), *CurrentMove->MoveID.ToString(), SeqIndex, TargetSequence.Num());

        if (SeqIndex == TargetSequence.Num())
        {
            bPrereqReady = true; // 序列完成
            UE_LOG(LogTemp, Warning, TEXT("[Seq] Prerequisite sequence COMPLETE for %s! Ready to trigger."), *CachedSpecialMove->MoveID.ToString());
        }
    }
    else if (SeqIndex > 0 && CurrentMove && !CurrentMove->bIgnoreForSequence) // CurrentMove 存在且不应被忽略
    {
        // 只有序列已经开始 (SeqIndex > 0) 且当前动作不匹配，并且当前动作不能被忽略时才重置
        UE_LOG(LogTemp, Log, TEXT("[Seq] Sequence broken by move %s (not ignored). Resetting progress."), *CurrentMove->MoveID.ToString());
        SeqIndex = 0;
        bPrereqReady = false;
    }
    // 若 SeqIndex == 0 且未匹配 → 忽略，允许在序列开始前的任意“杂招”
    // 若 CurrentMove->bIgnoreForSequence 为 true，即使不匹配且 SeqIndex > 0，也不重置 (例如闪避动作)
}
```

### 5.4 触发特殊招式 (例如在处理输入时)

```cpp
// 假设 Key 是当前输入, ECombatInputKey::CIK_Light 代表轻攻击
if (Key == ECombatInputKey::CIK_Light && bPrereqReady && CachedSpecialMove)
{
    UE_LOG(LogTemp, Log, TEXT("[Seq] Triggering Special Move %s!"), *CachedSpecialMove->MoveID.ToString());
    ExecuteSimulatedMove(CachedSpecialMove); // 或者实际的招式执行函数
    
    // 重置状态，以便可以再次准备触发 (如果设计如此)
    SeqIndex     = 0;
    bPrereqReady = false;
    // TargetSequence 通常不需要清除，除非 CachedSpecialMove 会改变
    return; // 消耗了输入，不再进行后续的普通招式判断
}
```

---

## 6. 招式示例（完整数据）

### 6.1 Flying Swallow 链（龙剑）

| 字段                      | 段 1                               | 段 2 / 3（Lv2+/Lv3） |
| ----------------------- | --------------------------------- | ----------------- |
| MoveID                  | FSwallow                          | FSwallow\_Chain   |
| InputPattern            | Jump+前+Heavy                      | Heavy             |
| ValidState              | Air                               | FS\_Rebound       |
| Startup/Active/Recovery | 8/3/18                            | 6/3/14            |
| ChainRule               | Max=武器等级，Window=12f，Trigger=OnHit | 同上                |
| TargetPolicy            | Nearest                           | PrevTarget        |
| Tags                    | Signature, GapCloser              | Signature         |

### 6.2 Izuna Grab / Izuna Drop

| 字段                      | Izuna\_Grab        | Izuna\_Drop |
| ----------------------- | ------------------ | ----------- |
| InputPattern            | Hold Heavy         | – (Auto)    |
| ValidState              | Air                | Throw       |
| Startup/Active/Recovery | 4/2/28             | 0/–/40      |
| ExecCondition           | TargetTag=Humanoid | –           |
| CancelCondition         | OnHit              | LockMove    |
| LifeSteal               | 8 %                | 12 %        |

### 6.3 地面轻连（三轻一重）

| 段 | MoveID    | Input | 帧数据 S/A/R | 窗口    | Next        |
| - | --------- | ----- | --------- | ----- | ----------- |
| 1 | Light\_A1 | Light | 8/6/10    | 18‑24 | A2          |
| 2 | Light\_A2 | Light | 7/5/11    | 17‑23 | A3, Asc\_Up |
| 3 | Light\_A3 | Light | 10/8/14   | 20‑28 | Fin\_H      |
| 4 | Fin\_H    | Heavy | 6/3/20    | –     | –           |

### 6.4 上挑分支 & 空连

| Move     | Input   | 状态     | 说明                  |
| -------- | ------- | ------ | ------------------- |
| Asc\_Up  | ↑+Heavy | Ground | Launcher ＋ AutoJump |
| AirJumpY | Heavy   | Air    | 空中轻斩                |
| AirDiveY | ↓+Heavy | Air    | 下砸 Finisher         |

---

## 7 高级系统（详细）

### 7.1 本质爆发技 Obliteration Technique（OT）

* **触发**：目标 `EnemyState=LimbLost`，玩家近距离 Heavy。
* **字段**：`ExecCondition=TargetState=LimbLost`、`OTVariant`、`LifeSteal`、`CameraCue=OT_KillCam`。
* **示例**：`OT_HeadSlice` – Startup 6f 无敌，LifeSteal 15 %，HitStop 10f。

### 7.2 终极技 Ultimate Technique（UT）

| Charge级 | 需求                | 伤害倍率 | AOE半径        | CameraCue |
| ------- | ----------------- | ---- | ------------ | --------- |
| Lv1     | Hold Heavy 40f    | ×2.0 | 3 m          | UT\_Lv1   |
| Lv2     | 70f 或吸 2 Essence  | ×3.0 | 4 m          | UT\_Lv2   |
| Lv3     | 100f 或吸 4 Essence | ×4.5 | 5 m + Launch | UT\_Lv3   |

* `EssenceAbsorb` 每吸 1 珠减 10 f，最高降至 50 f。

### 7.3 精准格挡 & 反击

* `ParryWindow=4` logic frames。
* 成功后 `CharacterState=ParrySuccess` 10 f 无敌；开放 `Counter_X/Y`，Damage ×1.5。

### 7.4 濒死增益 Near‑Death

* 触发：HP ≤ 15 % → `DamageScalar=+15 %`, `DefenseScalar=-10 %`, HUD 闪红。
* 生命周期：20 s 或 HP > 25 % 时终止。

### 7.5 Style 评分
`StyleScore += MoveValue × ComboDepth × (1‑RepeatPenalty)`；每 2 s 衰减 5 %。
Rank S ≥ 400，A ≥ 280，B ≥ 180，C ≥ 90。

---

## 8 相机动态系统（概览）

| 事件         | NG2 实际表现                 | 实现指针                                                                      |
| ---------- | ------------------------ | ------------------------------------------------------------------------- |
| **普通命中**   | 2‑3 帧 Hit Stop + 细微屏幕震   | MoveData 字段 `FXTag=Hit_Light` → CombatVFXSubsystem 触 `Shake(幅0.4,l=0.08)` |
| **重击/终结技** | 中幅 Shake + 0.75× 慢动作 6 帧 | `FXTag=Finisher` → 执行 `TimeDilation=0.75`                                 |
| **UT 释放**  | 摄像机拉远 + FOV +10°         | 在 UT MoveData `CameraCue=UT_Release`                                      |
| **OT**     | 时间冻结 10 f，局部放血特写         | `CameraCue=OT_KillCam`（高优先级）                                              |

> *注*：相机系统牵涉全局反馈，可独立 PRD；当前文档提供接口/Tag 规范即可，具体数值可在 **《战斗相机与反馈 PRD》** 单独规划。

---

## 9 敌人类型扩展

### 9.1 EnemyState

`Healthy / Stun / LimbLost / Crumple / BossSpecial`

### 9.2 OT 变体

| 敌人类别  | OT 动画参考 | DataAsset 标记          |
| ----- | ------- | --------------------- |
| 忍者系小型 | 斩首      | `OTVariant=HeadSlice` |
| 重装兵   | 断腰上斩    | `OTVariant=TorsoCut`  |
| 飞行魔物  | 抓落后地面突刺 | `OTVariant=AirStab`   |

### 9.3 Boss 交互预留

* `BossPhaseEvent` 字段：可在特定 HP 段触发特写 QTE。\*
* 大型 Boss 特殊 OT / 处决放至 **BossMoveSet** DataAsset ；当前版本仅留接口。

---

## 10 QA 测试矩阵（节选）

| TC ID     | Scenario                                  | Expected Result                                  |
| --------- | ----------------------------------------- | ---------------------------------------------- |
| CHAIN‑01  | 龙剑 Lv1 Jump→Y 命中→狂按 Y                 | 单段飞燕，落地                                    |
| CHAIN‑02  | 龙剑 Lv2 同上                               | 双燕成功                                        |
| OT‑01     | 断肢敌旁 Heavy                              | OT 触发，LifeSteal + 无敌验证                     |
| UT‑01     | Hold Heavy 100 f                          | UT Lv3 AOE；吸魂提前释放 ok                       |
| PARRY‑01  | 被击前 3 f Block                           | Parry 成功→Counter\_Y 极速触发                   |
| NEAR‑01   | HP 10 % 以下                              | Damage +15 % & HUD 闪红                          |
| STYLE‑01  | 15 Hit 不重复                              | StyleRank ≥ A                                  |
| BUFFER‑01 | 输入 8 keys / 6 f                         | FIFO pop oldest；无崩溢                            |
| FPS‑01    | Logic tick at 40 fps                      | 系统仍按 120 Hz 固定步执行                          |
| **PREREQ-POS-01 (原 POS-03)** | **输入: B3 (非序列) A1 A2 A3 Light (触发键)** | **预期: 前缀非序列动作允许 → 特殊招式 C 触发**          |
| **PREREQ-NEG-01 (原 NEG-05)** | **输入: A1 A2 B1 (非序列成员) A3 Light**    | **预期: 序列中插入非成员 (B1) → 特殊招式 C 不触发，序列重置** |
| CAM‑01    | UT 释放摄像机 FOV 检查                       | (根据《战斗相机与反馈 PRD》的具体数值检查)                 |
| OT‑VAR‑01 | 不同敌 OT Variant 正确匹配                    | (根据敌人类型和 OTVariant 设置检查)                  |


见英文版 §7；新增 **CAM‑01**（UT 释放摄像机 FOV 检查）、**OT‑VAR‑01**（不同敌 OT Variant 正确匹配）。

---

## 11 里程碑

| ID   | 日期    | 交付                                       |
| ---- | ----- | ---------------------------------------- |
| M0‑R | 06‑21 | 两级缓冲 + 链式原型                              |
| M1‑R | 07‑15 | Move/Chain 编辑器 + 格挡系统 + 相机接口基础           |
| M2‑R | 08‑10 | OT / UT / 敌状态系统 + StyleScore + 相机 Tag 完整 |
| M3‑R | 09‑20 | 爪／镰刀武器包 + Boss Phase 接口                  |

---

## 12 风险与缓解（更新）

OT 动画量高 → 先共用骨骼、后期替换；相机 Cue 资源冲突 → 统一 Tag 映射测试工具。

---

## 13 参数设计理念

* **Startup/Active/Recovery 比例**：近战轻击 1:1:1，重击 1:0.5:2；保持 NG2 节奏。
* **ComboWindow 宽度**：基本招式 6 f（100 ms），高段连击缩至 4 f；链式技专用 `ChainWindow` ≤ 12 f。
* **InputBuffer 长度**：6 f = 人体按键 ±50 ms 偏差；链式技额外 Local 2 键。

---

## 14 术语表

| 术语              | 解释                                    |
| --------------- | ------------------------------------- |
| **HitProxy**    | UE 碰撞检测体，用于攻击／受击盒精确判断                 |
| **WindRun**     | NG2 闪避技，8 f 无敌并位移；本项目归类于 Defensive 状态 |
| **OT**          | Obliteration Technique，本质爆发技，断肢终结     |
| **UT**          | Ultimate Technique，蓄力范围大招             |
| **ParryWindow** | 精准格挡有效时间窗口（逻辑帧）                       |
| **ChainRule**   | 数据字段，定义链式招式最大次数与窗口                    |
| **StyleScore**  | 连招多样性与技巧评分系统                          |
| **PrereqComboSeq** | DataAsset字段，定义招式触发所需的前置招式序列        |

---

## 15 备注

武器即刻切换、高成本 VFX、环境破坏待后续范围；相机系统如需深度可分拆独立 PRD。
