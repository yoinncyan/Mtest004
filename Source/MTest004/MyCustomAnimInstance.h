// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Animation/AnimInstanceProxy.h"
#include "Animation/AnimNode_SequencePlayer.h"
#include "Animation/BlendSpace.h"
#include "Animation/BlendSpace1D.h"
#include "MyCustomAnimInstance.generated.h"

/**
 * 
 */
UCLASS()
class MTEST004_API UMyCustomAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
	
public:
    // 缓存角色和运动组件的引用
    UPROPERTY(BlueprintReadOnly, Category = "Animation")
    ACharacter* OwningCharacter;

    UPROPERTY(BlueprintReadOnly, Category = "Animation")
    UCharacterMovementComponent* MovementComponent;

    // 动画变量
    UPROPERTY(BlueprintReadOnly, Category = "Animation")
    float GroundSpeedT;

    UPROPERTY(BlueprintReadOnly, Category = "Animation")
    bool ShouldMoveT;

    UPROPERTY(BlueprintReadOnly, Category = "Animation")
    bool IsFallingT;

    UPROPERTY(BlueprintReadOnly, Category = "Animation")
    bool IsinAirT;

    UPROPERTY(BlueprintReadOnly, Category = "Animation")
    FVector CurrentAccelerationT; // 当前加速度

    /** 当前播放的动画序列 */
    UPROPERTY(BlueprintReadOnly, Category = "Animation")
    UAnimSequence* CurrentAnimSequence;

    // 保存当前播放的动画序列
    UPROPERTY(BlueprintReadOnly, Category = "Animation")
    UAnimSequenceBase* CurrentSequence;

public:

    // 初始化时的逻辑
    virtual void NativeInitializeAnimation() override;

    // 每帧更新逻辑
    virtual void NativeUpdateAnimation(float DeltaSeconds) override;



    UFUNCTION(BlueprintCallable, Category = "Animation")
    UAnimSequence* GetPlayingAnimSequence(USkeletalMeshComponent* SkeletalMeshComp);

    UFUNCTION()
    void HandleNotifyEnd(FName NotifyName, const FBranchingPointNotifyPayload& Payload);

    UFUNCTION()
    void HandleNotifyBegin(FName NotifyName, const FBranchingPointNotifyPayload& Payload);
};
