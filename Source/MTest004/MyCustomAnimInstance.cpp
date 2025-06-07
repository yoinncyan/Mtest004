// Fill out your copyright notice in the Description page of Project Settings.


#include "MyCustomAnimInstance.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Character.h"
#include "Animation/AnimSingleNodeInstance.h"



void UMyCustomAnimInstance::NativeInitializeAnimation()
{
    // 调用父类的初始化方法
    Super::NativeInitializeAnimation();

    // 获取拥有者角色
    OwningCharacter = Cast<ACharacter>(TryGetPawnOwner());
    if (OwningCharacter)
    {
        // 缓存角色的 MovementComponent
        MovementComponent = OwningCharacter->GetCharacterMovement();
    }

    // 初始化
    CurrentSequence = nullptr;

    // 绑定“开始”通知
    OnPlayMontageNotifyBegin.AddDynamic(this, &UMyCustomAnimInstance::HandleNotifyBegin);
    //UE_LOG(LogTemp, Warning, TEXT("OnPlayMontageNotifyBegin binding successful."));

    // 绑定“结束”通知
    OnPlayMontageNotifyEnd.AddDynamic(this, &UMyCustomAnimInstance::HandleNotifyEnd);
    //UE_LOG(LogTemp, Warning, TEXT("OnPlayMontageNotifyEnd binding successful."));

}

void UMyCustomAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
    Super::NativeUpdateAnimation(DeltaSeconds);
    //UE_LOG(LogTemp, Warning, TEXT("NativeUpdateAnimation called."));

    if (!OwningCharacter || !MovementComponent)
    {
        return;
    }

    // 更新是否处于空中状态
    IsinAirT = MovementComponent->IsFalling();
    IsFallingT = MovementComponent->IsFalling();
    if (GroundSpeedT > 50.f)
    {
        ShouldMoveT = true;
    }
    else { ShouldMoveT = false; }
    if (!IsFallingT)
    {
        // 更新速度
        FVector Velocity = MovementComponent->Velocity;
        //GroundSpeedT = FVector::VectorPlaneProject(Velocity, FVector::UpVector).Size();
        GroundSpeedT = Velocity.Length();
        //UE_LOG(LogTemp, Warning, TEXT("GroundSpeedT = %f"),GroundSpeedT);
    }
}


UAnimSequence* UMyCustomAnimInstance::GetPlayingAnimSequence(USkeletalMeshComponent* SkeletalMeshComp)
{
    if (!SkeletalMeshComp)
    {
        UE_LOG(LogTemp, Warning, TEXT("SkeletalMeshComp is null!"));
        return nullptr;
    }

    // 如果动画模式是 AnimationBlueprint
    if (SkeletalMeshComp->GetAnimationMode() == EAnimationMode::AnimationBlueprint)
    {
        // 动画蓝图模式需要特定的操作去获取当前播放的序列
        UE_LOG(LogTemp, Log, TEXT("Animation mode is AnimationBlueprint. Currently no direct way to access playing sequence."));
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, TEXT("Animation mode is AnimationBlueprint. Currently no direct way to access playing sequence."));
        return nullptr;
    }

    // 如果动画模式是单节点模式
    if (SkeletalMeshComp->GetAnimationMode() == EAnimationMode::AnimationSingleNode)
    {
        return Cast<UAnimSequence>(SkeletalMeshComp->GetSingleNodeInstance()->GetCurrentAsset());
    }

    // 无法识别的动画模式
    UE_LOG(LogTemp, Warning, TEXT("Unknown animation mode."));
    return nullptr;
}

void UMyCustomAnimInstance::HandleNotifyBegin(FName NotifyName, const FBranchingPointNotifyPayload& Payload)
{
    //UE_LOG(LogTemp, Warning, TEXT("HandleNotifyBegin called with NotifyName: %s"), *NotifyName.ToString());
    //if (NotifyName == TEXT("Hit"))
    //{
    //    //UE_LOG(LogTemp, Error, TEXT("HandleNotifyBegin!!!!!!!!!!!!!!!!!!!!!"));
    //    if (auto* Char = Cast<AMTest002Character>(GetOwningActor()))
    //        if (auto* W = Char->GetPrimaryWeapon()) 
    //        {
    //            W->EnableHitDetection();
    //        }
    //}
}

void UMyCustomAnimInstance::HandleNotifyEnd(FName NotifyName, const FBranchingPointNotifyPayload& Payload)
{
    //UE_LOG(LogTemp, Error, TEXT("HandleNotifyEnd!!!!!!!!!!!!!!!!!!!!!"));
    //UE_LOG(LogTemp, Warning, TEXT("HandleNotifyEnd called with NotifyName: %s"), *NotifyName.ToString());

    //if (NotifyName == TEXT("Hit"))
    //{
    //    //UE_LOG(LogTemp, Error, TEXT("HandleNotifyEnd!!!!!!!!!!!!!!!!!!!!!"));
    //    if (auto* Char = Cast<AMTest002Character>(GetOwningActor()))
    //        if (auto* W = Char->GetPrimaryWeapon())
    //        {
    //            W->DisableHitDetection();
    //        }
    //}
}