// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "MeleeWeaponComponent.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Camera/PlayerCameraManager.h"
#include "WeaponBase.generated.h"

class AMTest004Character;

UCLASS()
class MTEST004_API AWeaponBase : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AWeaponBase();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void OnConstruction(const FTransform& Transform) override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	//UFUNCTION()
	//void OnMeleeHit(const FMeleeHitInfo& HitInfo);

	/** 近战武器碰撞组件 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
	UMeleeWeaponComponent* MeleeComp;

	/** 打开伤害检测（开启碰撞／Overlap） */
	UFUNCTION(BlueprintCallable, Category = "Weapon|Combat")
	void EnableHitDetection();

	/** 关闭伤害检测 */
	UFUNCTION(BlueprintCallable, Category = "Weapon|Combat")
	void DisableHitDetection();

	// 装备/卸下 接口
	virtual void EquipTo(class AMTest002Character* Character);
	virtual void Unequip();

	UFUNCTION()
	void ChangeAttState(AActor* OtherActor, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void Init();

	// 如果你希望在蓝图中拖放或在细节面板指定 SkeletalMesh
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Mesh")
	USkeletalMeshComponent* WeaponSkeletalMesh;

	// 如果你希望在蓝图中拖放或在细节面板指定 StaticMesh
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Mesh")
	UStaticMeshComponent* WeaponStaticMesh;

	void TestCamSnak();

/**
 * 触发一次静帧/慢动作
 * @param Type                        模式：完全冻结 or 慢动作
 * @param DelayedTrigger              延迟多长时间后触发（秒）
 * @param Duration                    效果持续时长（秒）
 * @param Rate                        时间缩放率（0=完全停，1=正常）
 * @param bSpeedCompensation          是否开启角色速度补偿
 * @param bSpeedCompensationBefore    是否在冻结前先补偿
 * @param SpeedCompensationDegree     补偿力度系数（0～1，1=完全按时间缩放，0=完全不动）
 */
	void OnHitFreezeFrameDetailed(
		EFreezeFrameType Type,
		float DelayedTrigger,
		float Duration,
		float Rate,
		bool  bSpeedCompensation,
		bool  bSpeedCompensationBefore,
		float SpeedCompensationDegree
	);

	/** 真正执行冻结的函数 */
	//UFUNCTION()
	//void DoFreezeFrame();

	/** FTSTicker 回调签名必须返回 bool */
	//bool DoRestoreFrame(float DeltaTime);

	FTimerHandle DelayHandle;

	/** FTSTicker 的 DelegateHandle */
	FTSTicker::FDelegateHandle RestoreHandle;

	// 缓存参数
	EFreezeFrameType CachedType;
	float CachedRate;
	float CachedDuration;
	bool  bCachedSpeedComp;
	bool  bCachedBeforeFreeze;
	float CachedCompDegree;
	float OriginalGlobalDil;
	FVector OriginalVelocity;
	float   OriginalCustomDil;

	bool bIsCurrentlyFreezing = false;
};

UENUM(BlueprintType)
enum class EFreezeFrameType : uint8
{
	FullFreeze UMETA(DisplayName = "完全冻结"),   // TimeDilation = 0
	SlowMotion UMETA(DisplayName = "慢动作"),     // TimeDilation = Rate
};
