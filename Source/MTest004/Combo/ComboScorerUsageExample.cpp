/**
 * 这是一个使用示例文件，展示如何配置和使用连招评分系统
 * 这个文件主要用于参考，不一定需要编译到最终项目中
 */

#include "ComboScorerUsageExample.h"
#include "CombatComponent.h"
#include "CombatInputBuffer.h"
#include "ComboMoveData.h"
#include "ComboScorer.h"
#include "FlyingSwallowScorer.h"

void UComboScorerUsageExample::ExampleUsage()
{
    /**
     * === 使用示例 1：在 DataAsset 中配置评分器 ===
     * 
     * 1. 创建一个 ComboMoveData DataAsset
     * 2. 在 "Scoring" 分类下：
     *    - CustomScorerClass = UFlyingSwallowScorer (对于飞燕招式)
     *    - BaseScore = 120.0f (比普通招式高一些)
     * 3. 在 "Input" 分类下：
     *    - InputPattern = [CIK_Jump, CIK_Light] (跳跃+轻攻击)
     *    - InputFlexBits = 0b01 (第一个输入允许灵活性)
     */

    /**
     * === 使用示例 2：在运行时调用评分系统 ===
     */
    /*
    // 假设我们有一个 CombatComponent 和 InputBuffer
    UCombatComponent* CombatComp = GetOwner()->FindComponentByClass<UCombatComponent>();
    UCombatInputBuffer* InputBuffer = GetOwner()->FindComponentByClass<UCombatInputBuffer>();
    
    if (CombatComp && InputBuffer)
    {
        // 选择最佳匹配的招式
        const UComboMoveData* BestMove = CombatComp->SelectBestMoveFromBuffer(InputBuffer);
        
        if (BestMove)
        {
            // 执行选中的招式
            CombatComp->AttemptCombo(BestMove);
        }
    }
    */

    /**
     * === 评分公式说明 ===
     * 
     * Score = BaseScore + OrderBonus – ΔtPenalty + FlexBonus
     * 
     * - BaseScore: 在 ComboMoveData 中配置的基础分数
     * - OrderBonus: 在输入缓冲区中越早找到的模式得分越高
     * - ΔtPenalty: 输入序列时间过长会被扣分
     * - FlexBonus: 使用灵活输入会获得额外分数
     * 
     * 特殊评分器（如 FlyingSwallowScorer）会在此基础上添加额外逻辑
     */

    /**
     * === 自定义评分器开发指南 ===
     * 
     * 1. 继承 UComboScorer 类
     * 2. 重写 CalculateScore_Implementation 函数
     * 3. 在函数开始时调用 Super::CalculateScore_Implementation(Context)
     * 4. 基于特定招式需求添加额外的评分逻辑
     * 5. 在 ComboMoveData 的 CustomScorerClass 中指定你的评分器类
     */
}
