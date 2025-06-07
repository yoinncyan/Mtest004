// Fill out your copyright notice in the Description page of Project Settings.


#include "WeaponBase.h"
#include "Engine/World.h"
//#include "MTAiCharacter000.h"
//#include "MassSignalSubsystem.h"
//#include "KeyBoneFinderComponent.h"
//#include "PostureAdjustComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Camera/CameraShakeBase.h"
#include "GameFramework/WorldSettings.h"
#include "GameFramework/Character.h"
#include "TimerManager.h"
#include "Containers/Ticker.h"

DEFINE_LOG_CATEGORY_STATIC(LogMeleeFreezeDetail, Log, All);

// Sets default values
AWeaponBase::AWeaponBase()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	PrimaryActorTick.bCanEverTick = false;
	WeaponStaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WeaponStaticMesh"));
	SetRootComponent(WeaponStaticMesh);

	// 默认都隐藏，由 EquipTo/Unequip 接口控制显隐
	//WeaponSkeletalMesh->SetHiddenInGame(true);
	WeaponStaticMesh->SetHiddenInGame(true);
}

// Called when the game starts or when spawned
void AWeaponBase::BeginPlay()
{
	Super::BeginPlay();
	// 确保在 BeginPlay 后依然使用最新的尺寸
	OnConstruction(GetActorTransform());
	Init();
}

// Called every frame
void AWeaponBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AWeaponBase::EquipTo(AMTest002Character* Character)
{
	if (WeaponStaticMesh)
	{
		WeaponStaticMesh->SetHiddenInGame(false);
	}
}

void AWeaponBase::Unequip()
{
	if (WeaponStaticMesh)
	{
		WeaponStaticMesh->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
		WeaponStaticMesh->SetHiddenInGame(true);
	}
}
void AWeaponBase::ChangeAttState(AActor* OtherActor, bool bFromSweep, const FHitResult& SweepResult)
{

}

void AWeaponBase::Init()
{
	MeleeComp = FindComponentByClass<UMeleeWeaponComponent>();
	if (MeleeComp)
	{
		//MeleeComp->OnMeleeHit.AddDynamic(this, &AWeaponBase::OnMeleeHit);
		MeleeComp->EnableHitBox(false); // 默认关闭
	}
}
void AWeaponBase::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
}
void AWeaponBase::EnableHitDetection()
{
	if (MeleeComp)
	{
		// 打开碰撞检测
		MeleeComp->EnableHitBox(true);
	}
}
void AWeaponBase::DisableHitDetection()
{
	if (MeleeComp)
	{
		// 关闭碰撞检测
		MeleeComp->EnableHitBox(false);
	}
}

void AWeaponBase::TestCamSnak()
{
	UWorld* World = GetWorld();
	if (!World) return;
}
void AWeaponBase::OnHitFreezeFrameDetailed(EFreezeFrameType Type, float DelayedTrigger, float Duration, float Rate, bool  bSpeedCompensation, bool  bSpeedCompensationBefore, float SpeedCompensationDegree)
{

}