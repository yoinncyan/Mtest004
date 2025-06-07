// Copyright Epic Games, Inc. All Rights Reserved.

#include "MTest004Character.h"
#include "Engine/LocalPlayer.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "Combo/CombatInputBuffer.h"
#include "Combo/ComboScorer.h"
#include "Combo/PRDComplianceValidator.h"




DEFINE_LOG_CATEGORY(LogTemplateCharacter);

//////////////////////////////////////////////////////////////////////////
// AMTest004Character

AMTest004Character::AMTest004Character()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
		
	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // ...at this rotation rate

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm
	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named ThirdPersonCharacter (to avoid direct content references in C++)

	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;
	USkeletalMeshComponent* MeshComp = GetMesh();
	UCharacterMovementComponent* MoveComp = GetCharacterMovement();
	if (MeshComp && MoveComp)
	{
		// 1) 还原 Movement 的默认组（PrePhysics），保证它先处理移动输入和根运动
		MoveComp->PrimaryComponentTick.TickGroup = TG_PrePhysics;
		MoveComp->PrimaryComponentTick.bCanEverTick = true;  // 确保开启

		// 2) 把 Mesh 放到 PostPhysics，这样动画在物理后、移动后才更新，不再循环
		MeshComp->PrimaryComponentTick.TickGroup = TG_PostPhysics;
		MeshComp->PrimaryComponentTick.bCanEverTick = true;  // 保留动画更新

		// 3) 从彼此移除依赖
		MeshComp->PrimaryComponentTick.RemovePrerequisite(MoveComp, MoveComp->PrimaryComponentTick);
		MoveComp->PrimaryComponentTick.RemovePrerequisite(MeshComp, MeshComp->PrimaryComponentTick);
	}
}

void AMTest004Character::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();

	InitWeapon();
	EquipPrimary();
}

//////////////////////////////////////////////////////////////////////////
// Input

void AMTest004Character::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// Add Input Mapping Context
	if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}
	
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {
		
		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AMTest004Character::Move);

		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AMTest004Character::Look);

		// Left Mouse Click
		EnhancedInputComponent->BindAction(LeftMouseClickAction, ETriggerEvent::Triggered, this, &AMTest004Character::LeftMouseClick);

		// Left Mouse Click
		EnhancedInputComponent->BindAction(RightMouseClickAction, ETriggerEvent::Triggered, this, &AMTest004Character::RightMouseClick);
	}
	else
	{
		UE_LOG(LogTemplateCharacter, Error, TEXT("'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}

void AMTest004Character::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	
		// get right vector 
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// add movement 
		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

void AMTest004Character::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void AMTest004Character::LeftMouseClick(const FInputActionValue& Value)
{	if (Controller != nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("成功点击鼠标左键"));
	}
}

void AMTest004Character::RightMouseClick(const FInputActionValue& Value)
{	if (Controller != nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("成功点击鼠标右键"));
	}
}

void AMTest004Character::InitWeapon()
{
	if (PrimaryWeaponClass)
	{
		PrimaryWeapon = GetWorld()->SpawnActor<AWeaponBase>(PrimaryWeaponClass);
		if (PrimaryWeapon)
		{
			UE_LOG(LogTemp, Warning, TEXT("成功生成武器实例"));
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("武器实例生成失败"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("PrimaryWeaponClass 未设置"));
	}
}
void AMTest004Character::EquipPrimary()
{
	if (!PrimaryWeaponClass) return;

	// 假设角色手骨骼名是 "hand_r"
	static const FName BoneName(TEXT("weapon_r"));
	PrimaryWeapon->GetRootComponent()->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetIncludingScale, BoneName);
	PrimaryWeapon->WeaponStaticMesh->SetHiddenInGame(false);
}