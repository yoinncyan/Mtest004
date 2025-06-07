// Fill out your copyright notice in the Description page of Project Settings.


#include "MeleeWeaponComponent.h"

                                // Sets default values for this component's properties
UMeleeWeaponComponent::UMeleeWeaponComponent()
{
    // Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
    // off to improve performance if you don't need them.
    PrimaryComponentTick.bCanEverTick = false;

    // 默认关闭
    SetCollisionEnabled(ECollisionEnabled::NoCollision);
    // 只对 Pawn 产生 Overlap
    SetCollisionObjectType(ECC_Pawn);
    SetCollisionResponseToAllChannels(ECR_Ignore);
    SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
}


// Called when the game starts
void UMeleeWeaponComponent::BeginPlay()
{
    Super::BeginPlay();
    OnComponentBeginOverlap.AddDynamic(this, &UMeleeWeaponComponent::OnHitBoxBeginOverlap);
}

void UMeleeWeaponComponent::EnableHitBox(bool bEnable)
{
    SetCollisionEnabled(bEnable ? ECollisionEnabled::QueryOnly : ECollisionEnabled::NoCollision);
}

void UMeleeWeaponComponent::OnHitBoxBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    // 1) 先排除：自己 和 空指针
    if (!OtherActor || OtherActor == GetOwner())
    {
        return;
    }

    // 2) 仅对指定目标类型生效
    //if (!OtherActor->IsA(AMTAiCharacter000::StaticClass()))
    //{
    //    return;
    //}

    // 1) get the approximate impact point: use this component's location as entry
    FVector InPoint = GetComponentLocation();

    // compute the closest point on the other collider
    FVector ClosestPt;
    OverlappedComp->GetClosestPointOnCollision(InPoint, ClosestPt);

    //if (OtherActor->IsA(AMTAiCharacter000::StaticClass()))
    //{
    //    // 2) if the other has a CapsuleComponent, clamp inside its volume
    //    if (UCapsuleComponent* Cap = OtherActor->FindComponentByClass<UCapsuleComponent>())
    //    {
    //        const FVector Origin = Cap->GetComponentLocation();
    //        const FVector UpDir = Cap->GetUpVector();
    //        const float   HalfH = Cap->GetScaledCapsuleHalfHeight();
    //        const float   Radius = Cap->GetScaledCapsuleRadius();

    //        // segment AB is the capsule line
    //        const FVector A = Origin + UpDir * HalfH;
    //        const FVector B = Origin - UpDir * HalfH;
    //        const FVector Q = FMath::ClosestPointOnSegment(ClosestPt, A, B);
    //        FVector V = ClosestPt - Q;

    //        if (V.SizeSquared() > Radius * Radius)
    //        {
    //            V = V.GetSafeNormal() * Radius;
    //            ClosestPt = Q + V;
    //        }
    //    }

    //    UE_LOG(LogTemp, Error, TEXT("OtherActor Is True!!! 黄点测试"));

    //    // 3) debug draw
    //    DrawDebugSphere(GetWorld(), ClosestPt, 8.0f, 8, FColor::Yellow, false, 2.0f);
    //}

    // 4) broadcast refined hit info
    FMeleeHitInfo Info;
    Info.Target = OtherActor;
    Info.WeaponTag = WeaponTag;
    Info.WeaponType = WeaponType;
    Info.HitLocation = ClosestPt;
    Info.HitResult = SweepResult;
    OnMeleeHit.Broadcast(Info);
}


// Called every frame
void UMeleeWeaponComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // ...
}

