// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/BoxComponent.h"
#include "Components/ActorComponent.h"
#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "MeleeWeaponComponent.generated.h"

/**
 * 
 */
 /**
  * Information about a melee hit event.
  */
UENUM(BlueprintType)
enum class EMeleeWeaponType : uint8
{
    Sword,
    Axe,
    Hammer,
    Dagger,
    // … 
};
USTRUCT(BlueprintType)
struct FMeleeHitInfo
{
    GENERATED_BODY()
    UPROPERTY(BlueprintReadOnly) AActor* Target = nullptr;
    UPROPERTY(BlueprintReadOnly) FName WeaponTag;
    UPROPERTY(BlueprintReadOnly) EMeleeWeaponType WeaponType;
    UPROPERTY(BlueprintReadOnly) FHitResult HitResult;
    /** The point on the target's collision that was hit (clamped inside capsule if present) */
    UPROPERTY(BlueprintReadOnly)
    FVector HitLocation = FVector::ZeroVector;
};
/** Delegate for melee hit events */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMeleeHitSignature, const FMeleeHitInfo&, HitInfo);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class MTEST004_API UMeleeWeaponComponent : public UBoxComponent
{
	GENERATED_BODY()
	
public:
    // Sets default values for this component's properties
    UMeleeWeaponComponent();

    // 在 Details 面板里为每个组件实例选一个类型
    UPROPERTY(EditAnywhere, Category = "Melee")
    EMeleeWeaponType WeaponType = EMeleeWeaponType::Sword;

    UPROPERTY(EditAnywhere, Category = "Melee")
    FName WeaponTag;

    /** Fired when this weapon hits a valid target */
    UPROPERTY(BlueprintAssignable, Category = "Melee")
    FMeleeHitSignature OnMeleeHit;

    /** Enable or disable hit detection */
    UFUNCTION(BlueprintCallable, Category = "Melee")
    void EnableHitBox(bool bEnable);


protected:
    // Called when the game starts
    virtual void BeginPlay() override;

public:
    // Called every frame
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;


private:
    /** Handler for the BoxComponent overlap event */
    UFUNCTION()
    void OnHitBoxBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
};
