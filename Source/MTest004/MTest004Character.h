// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "Weapon/WeaponBase.h"
#include "Combo/ComboMoveData.h"
#include "Combo/CombatComponent.h"
#include "Combo/CombatInputBuffer.h"
#include "MTest004Character.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
struct FInputActionValue;

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

UCLASS(config=Game)
class AMTest004Character : public ACharacter
{
	GENERATED_BODY()

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;
	
	/** MappingContext */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputMappingContext* DefaultMappingContext;

	/** Jump Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* JumpAction;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* MoveAction;

	/** Look Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* LookAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* LeftMouseClickAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* RightMouseClickAction;

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Combo,meta = (AllowedClasses = "ComboMoveData"))
	TArray<TSoftObjectPtr<UComboMoveData>> ComboMoves;

public:

	// 三个可在编辑器里选择的武器蓝图类
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapons")
	TSubclassOf<AWeaponBase> PrimaryWeaponClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapons")																			 
	TSubclassOf<AWeaponBase> SecondaryWeaponClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapons")
	TSubclassOf<AWeaponBase> TertiaryWeaponClass;

	// 持有的武器实例指针
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapons")
	AWeaponBase* PrimaryWeapon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapons")
	AWeaponBase* SecondaryWeapon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapons")
	AWeaponBase* TertiaryWeapon;

	/** 获取当前武器实例 */
	UFUNCTION(BlueprintCallable, Category = "Weapons")
	AWeaponBase* GetPrimaryWeapon() const { return PrimaryWeapon; }

	AMTest004Character();
	

protected:

	/** Called for movement input */
	void Move(const FInputActionValue& Value);

	/** Called for looking input */
	void Look(const FInputActionValue& Value);

	void LeftMouseClick(const FInputActionValue& Value);

	void RightMouseClick(const FInputActionValue& Value);
			

protected:
	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
	// To add mapping context
	virtual void BeginPlay();

public:
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }

	//举例：切换武器的方法
	UFUNCTION(BlueprintCallable, Category = "Weapons")
	void EquipPrimary();

	UFUNCTION()
	void InitWeapon();
};
